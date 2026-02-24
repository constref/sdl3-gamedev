#include "d3d12rendersystem.h"

#include <logger.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <SDL3/SDL_system.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#define DXCHK(result, msg) \
    if (FAILED(result)) { \
        Logger::error(this, msg); \
        return false; \
    }

using namespace Microsoft::WRL;
using namespace d3d12rs;

D3D12RenderSystem::D3D12RenderSystem(Services &services, SDL_Window *window, int width, int height, int logW, int logH) : System(services)
{
	m_width = width;
	m_height = height;
	m_logW = logW;
	m_logH = logH;
	if (Config::IsStandaloneMode())
	{
		m_hWnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		assert(m_hWnd && "Unable to acquire HWND for provided window");
	}
}

D3D12RenderSystem::~D3D12RenderSystem()
{
	shutdown();
}

bool D3D12RenderSystem::initialize()
{
	ComPtr<ID3D12Debug> debugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
	debugInterface->EnableDebugLayer();

	ComPtr<IDXGIFactory4> dxgiFactory;
#if defined(_DEBUG)
	UINT factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else 
	UINT factoryFlags = 0;
#endif

	DXCHK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dxgiFactory)), "Couldn't create DXGIFactory2");

	// find an appropriate adapter
	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	size_t maxDedicatedVRam = 0;

	for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		dxgiAdapter1->GetDesc1(&adapterDesc);

		if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
			SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
			adapterDesc.DedicatedVideoMemory > maxDedicatedVRam)
		{
			maxDedicatedVRam = adapterDesc.DedicatedVideoMemory;
			DXCHK(dxgiAdapter1.As(&m_dxgiAdapter), "Coudn't acquire the DXGIAdapter4");
		}
	}

	// create the D3D device
	DXCHK(D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "Error creating the D3D12Device");

	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(m_device.As(&infoQueue)))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		std::array severities{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		std::array denyIds{
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		};

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumSeverities = severities.size();
		filter.DenyList.pSeverityList = severities.data();
		filter.DenyList.NumIDs = denyIds.size();
		filter.DenyList.pIDList = denyIds.data();
		DXCHK(infoQueue->PushStorageFilter(&filter), "Error setting queue filter");
	}

	// create the command queue
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	DXCHK(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)), "Couldn't create the command queue");

	// check if tearing is supported
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		bool allowTearing = false;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (SUCCEEDED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			{
				m_allowTearing = allowTearing;
			}
		}
	}

	createSwapchain();

	DXCHK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
		"Couldn't create the command allocator");

	DXCHK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)),
		"Couldn't create the command list");
	DXCHK(m_commandList->Close(), "Couldn't close command list");

	DXCHK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create fence");
	m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_fenceEvent && "Failed to create fence event");

	return true;
}

void d3d12rs::D3D12RenderSystem::shutdown()
{
	// flush the GPU, wait for one final fence value
	m_fenceValue++;
	if (FAILED(m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue)))
	{
		return;
	}
	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
		{
			::WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}
}

void d3d12rs::D3D12RenderSystem::beginFrame()
{
	m_frameResIndex = m_frameIndex % MaxFrames;
	auto &res = m_frameResources[m_frameResIndex];

	if (m_fence->GetCompletedValue() < res.fenceValue)
	{
		if (FAILED(m_fence->SetEventOnCompletion(res.fenceValue, m_fenceEvent)))
		{
			Logger::error(this, "Unable to set fence completion event");
			return;
		}
		::WaitForSingleObject(m_fenceEvent, UINT_MAX);
	}
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);
	D3D12_RESOURCE_BARRIER barrier{
		.Transition {.pResource = res.backbuffer.Get(),
		.StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET} };
	m_commandList->ResourceBarrier(1, &barrier);

	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_frameResIndex * m_RTVDescriptorSize;

	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void d3d12rs::D3D12RenderSystem::endFrame()
{
	auto &res = m_frameResources[m_frameResIndex];

	D3D12_RESOURCE_BARRIER barrier{
		.Transition {.pResource = res.backbuffer.Get(),
		.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PRESENT} };
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();

	std::array<ID3D12CommandList *, 1> commandLists{ m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(commandLists.size(), commandLists.data());

	UINT presentFlags = m_allowTearing && m_syncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;
	m_swapchain->Present(m_syncInterval, presentFlags);

	res.fenceValue = ++m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), res.fenceValue);

	m_frameIndex++;
}

void D3D12RenderSystem::update(Node &node)
{
}

void D3D12RenderSystem::updateTextures()
{
}

bool d3d12rs::D3D12RenderSystem::createSwapchain()
{
	Logger::info(this, "Creating and initializing swapchain resources");
	ComPtr<IDXGISwapChain4> dxgiSwapchain;
	ComPtr<IDXGIFactory4> dxgiFactory4;

	UINT factoryFlags = 0;
#if defined(_DEBUG)
	factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	DXCHK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dxgiFactory4)), "Couldn't create DXGIFactory2 during swapchain init");

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = m_width;
	swapchainDesc.Height = m_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = FALSE;
	swapchainDesc.SampleDesc = { 1, 0 };
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = MaxFrames;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapchain;
	DXCHK(dxgiFactory4->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hWnd, &swapchainDesc, nullptr, nullptr, &swapchain), "Failed to create a swapchain for the given HWND");
	DXCHK(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER), "Failed to disable full-screen shortcut.");
	DXCHK(swapchain.As(&m_swapchain), "Error getting swapchain");

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = MaxFrames;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DXCHK(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap)), "Unable to create descriptor heap");

	m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < MaxFrames; ++i)
	{
		auto &res = m_frameResources[i];
		DXCHK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&res.backbuffer)), "Unable to get swapchain back buffer");
		m_device->CreateRenderTargetView(res.backbuffer.Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_RTVDescriptorSize;
	}

	return true;
}
