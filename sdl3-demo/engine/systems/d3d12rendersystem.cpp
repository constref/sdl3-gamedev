#include "d3d12rendersystem.h"

#include <logger.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <SDL3/SDL_system.h>
#include <d3dcompiler.h>

using namespace DirectX;

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

	for (int i = 0; i < FramesInFlight; ++i)
	{
		auto &res = m_frameResources[i];
		DXCHK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&res.commandAllocator)),
			"Couldn't create the command allocator");
		DXCHK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, res.commandAllocator.Get(), nullptr, IID_PPV_ARGS(&res.commandList)),
			"Couldn't create the command list");
		DXCHK(res.commandList->Close(), "Couldn't close command list");
	}

	// create single use command objects
	DXCHK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_singleUseCommandAllocator.GetAddressOf())),
		"Couldn't create the single-use command allocator");
	DXCHK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_singleUseCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_singleUseCommandList.GetAddressOf())),
		"Couldn't create the single-use command list");
	DXCHK(m_singleUseCommandList->Close(), "Couldn't close command list");

	DXCHK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create fence");
	m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_fenceEvent && "Failed to create fence event");

	createSwapchain();
	m_copyOperations.reserve(MaxCopyOps);

	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(StagingBuffSize);
	DXCHK(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_stagingBuffer.GetAddressOf())),
		"Unable to create the main staging buffer");

	CD3DX12_RANGE readRange(0, 0);
	DXCHK(m_stagingBuffer->Map(0, &readRange, &m_stagingPtr), "Unable to map staging buffer");

	loadAssets();
	return true;
}

void d3d12rs::D3D12RenderSystem::loadAssets()
{
	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	Vertex vertices[] =
	{

	  { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1, 0, 0, 1) },
	  { XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0, 1, 0, 1) },
	  { XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(0, 0, 1, 1) },
	  { XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(1, 1, 0, 1) },
	  { XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 1, 1) },
	  { XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(0, 1, 1, 1) },
	  { XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(1, 1, 0, 1) },
	  { XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 0, 1) }
	};

	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
	auto buffState = D3D12_RESOURCE_STATE_COMMON;
	HRESULT result1 = m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &resDesc, buffState, nullptr, IID_PPV_ARGS(boxVerts.GetAddressOf()));
	copyBuffer(std::span<uint8_t>(reinterpret_cast<uint8_t*>(vertices), _countof(vertices)), boxVerts.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

void d3d12rs::D3D12RenderSystem::shutdown()
{
	// flush the GPU, wait for one final fence value
	m_fenceValue++;
	if (FAILED(m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue)))
	{
		return;
	}
	flushGPU();

	if (m_stagingBuffer)
	{
		m_stagingBuffer->Unmap(0, nullptr);
	}
	CloseHandle(m_fenceEvent);
}

bool d3d12rs::D3D12RenderSystem::createPipelineStateObject()
{
	return false;
}

bool d3d12rs::D3D12RenderSystem::createShaders()
{
	return false;
}

void d3d12rs::D3D12RenderSystem::beginFrame()
{
	m_frameResIndex = m_frameIndex % FramesInFlight;
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

	res.renderTargetIndex = m_swapchain->GetCurrentBackBufferIndex();
	res.commandList->Reset(res.commandAllocator.Get(), nullptr);

	// process pending copy operations
	if (m_copyOperations.size())
	{
		static std::vector<D3D12_RESOURCE_BARRIER> barriers(MaxCopyOps);
		barriers.clear();
		for (int i = 0; i < m_copyOperations.size(); ++i)
		{
			auto &copyOP = m_copyOperations[i];
			res.commandList->CopyBufferRegion(copyOP.dstBuffer.Get(), 0, m_stagingBuffer.Get(), copyOP.stagingOffset, copyOP.byteSize);

			if (copyOP.dstStateAfter != D3D12_RESOURCE_STATE_COMMON)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(copyOP.dstBuffer.Get(), copyOP.dstStateBefore, copyOP.dstStateAfter));
			}
		}
		res.commandList->ResourceBarrier(barriers.size(), barriers.data());
		m_copyOperations.clear();
	}

	// viewport and scissor setup
	D3D12_VIEWPORT viewport{ .TopLeftX = 0, .TopLeftY = 0, .Width = static_cast<float>(m_width), .Height = static_cast<float>(m_height), .MinDepth = 0, .MaxDepth = 1 };
	res.commandList->RSSetViewports(1, &viewport);

	auto rtBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[res.renderTargetIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	res.commandList->ResourceBarrier(1, &rtBarrier);

	FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(res.renderTargetIndex * m_descriptorSizes.RTV);
	res.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	const float aspectRation = m_width / static_cast<float>(m_height);
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRation, 0.1f, 100.0f);
}

void d3d12rs::D3D12RenderSystem::endFrame()
{
	auto &res = m_frameResources[m_frameResIndex];

	auto presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[res.renderTargetIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	res.commandList->ResourceBarrier(1, &presentBarrier);
	res.commandList->Close();

	std::array<ID3D12CommandList *, 1> commandLists{ res.commandList.Get() };
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
	swapchainDesc.Format = SwapchainFormat;
	swapchainDesc.Stereo = FALSE;
	swapchainDesc.SampleDesc = { 1, 0 };
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = RenderTargetCount;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapchain;
	DXCHK(dxgiFactory4->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hWnd, &swapchainDesc, nullptr, nullptr, &swapchain), "Failed to create a swapchain for the given HWND");
	DXCHK(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER), "Failed to disable full-screen shortcut.");
	DXCHK(swapchain.As(&m_swapchain), "Error getting swapchain");

	// RTV Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = RenderTargetCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DXCHK(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVDescriptorHeap)), "Unable to create RTV descriptor heap");
	// DSV Heap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DXCHK(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVDescriptorHeap)), "Unable to create DSV descriptor heap");

	// create the render target views
	m_descriptorSizes.RTV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < RenderTargetCount; ++i)
	{
		DXCHK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(m_backBuffers[i].GetAddressOf())), "Unable to get swapchain back buffer");
		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(m_descriptorSizes.RTV);
	}

	// create a depth/stencil view
	D3D12_RESOURCE_DESC dsDesc{};
	dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsDesc.Alignment = 0;
	dsDesc.Width = m_width;
	dsDesc.Height = m_height;
	dsDesc.DepthOrArraySize = 1;
	dsDesc.MipLevels = 1;
	dsDesc.Format = DepthStencilFormat;
	dsDesc.SampleDesc = { 1, 0 };
	dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE dsClearVal{};
	dsClearVal.Format = DepthStencilFormat;
	dsClearVal.DepthStencil = { 1, 0 };

	CD3DX12_HEAP_PROPERTIES dsHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXCHK(m_device->CreateCommittedResource(&dsHeapProps, D3D12_HEAP_FLAG_NONE, &dsDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &dsClearVal, IID_PPV_ARGS(m_depthStencil.GetAddressOf())),
		"Unable to create depth resource");

	m_descriptorSizes.DSV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, dsvHandle);

	return true;
}

void d3d12rs::D3D12RenderSystem::flushGPU()
{
	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
		{
			Logger::error(this, "Unable to set fence completion event");
			return;
		}
		::WaitForSingleObject(m_fenceEvent, UINT_MAX);
	}
}

uint32_t d3d12rs::D3D12RenderSystem::stageData(void *srcPtr, size_t byteSize, size_t alignment)
{
	size_t alignedSize = (byteSize + alignment - 1) & ~(alignment - 1);
	assert(m_stagingOffset + alignedSize < StagingBuffSize && "Not enough room in staging buffer for the copy operation");
	memcpy(static_cast<uint8_t *>(m_stagingPtr) + m_stagingOffset, srcPtr, byteSize);
	uint32_t dataOffset = m_stagingOffset;
	m_stagingOffset += alignedSize;
	return dataOffset;
}

void d3d12rs::D3D12RenderSystem::copyBuffer(std::span<uint8_t> srcData, ComPtr<ID3D12Resource> dst,
	D3D12_RESOURCE_STATES dstStateBefore, D3D12_RESOURCE_STATES dstStateAfter)
{
	constexpr uint32_t VertexIndexAlignment = 4;

	assert(m_copyOperations.size() < MaxCopyOps && "Copy operations buffer at capacity");
	m_copyOperations.push_back(CopyOperation{
		.stagingOffset = stageData(srcData.data(), srcData.size_bytes(), VertexIndexAlignment),
		.byteSize = srcData.size_bytes(),
		.dstBuffer = dst,
		.dstStateBefore = dstStateBefore,
		.dstStateAfter = dstStateAfter });
}
