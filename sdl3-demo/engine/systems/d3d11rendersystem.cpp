#include "d3d11rendersystem.h"

bool d3d11rs::D3D11RenderSystem::initialize(uint32_t backBufferCount)
{
	HRESULT result;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGIFactory2 *factory = nullptr;
	result = CreateDXGIFactory2(creationFlags, IID_PPV_ARGS(&factory));
	if (FAILED(result))
	{
		return false;
	}

	IDXGIAdapter *adapter;
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	IDXGIOutput *adapterOutput;
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	unsigned int numModes = 0;
	result = adapterOutput->GetDisplayModeList(pixelFormat, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	std::vector<DXGI_MODE_DESC> modeDescList;
	modeDescList.resize(numModes);
	result = adapterOutput->GetDisplayModeList(pixelFormat, DXGI_ENUM_MODES_INTERLACED, &numModes, modeDescList.data());

	unsigned int preferredModeIdx = 0;
	for (auto &desc : modeDescList)
	{
		if (desc.Width == width && desc.Height == height)
		{
			break;
		}
		preferredModeIdx++;
	}

	DXGI_ADAPTER_DESC adapterDesc;
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}
	videoMemMb = adapterDesc.DedicatedVideoMemory / 1024 / 1024;
	videoCardDesc = adapterDesc.Description;

	std::array featureLevels = { D3D_FEATURE_LEVEL_11_1 };
	D3D_FEATURE_LEVEL featureLevel;

	UINT deviceFlags = 0;
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, featureLevels.data(),
		static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION, &device, &featureLevel, &immediateCtx);
	if (FAILED(result))
	{
		return false;
	}

	// multisampling config
	DXGI_SAMPLE_DESC sampleDesc{ 0 };
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	if (!textureSharing)
	{
		//// create the swapchain
		//DXGI_SWAP_CHAIN_DESC1 swapChainDesc{0};
		//swapChainDesc.Width = width;
		//swapChainDesc.Height = height;
		//swapChainDesc.Stereo = false;
		//swapChainDesc.BufferCount = 2;
		//swapChainDesc.Format = pixelFormat;
		//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		//swapChainDesc.Format = pixelFormat;
		//swapChainDesc.Scaling = DXGI_SCALING_NONE;
		//swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		//swapChainDesc.Flags = 0;
		//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		//swapChainDesc.SampleDesc = sampleDesc;

		//this->vsync = vsync;
		//if (vsync)
		//{
		//    syncInterval = 1;
		//}

		//result = factory->CreateSwapChainForHwnd(device, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
		//if (FAILED(result))
		//{
		//    return false;
		//}
	}
	else
	{
		renderTargets.resize(backBufferCount);

		for (auto &target : renderTargets)
		{
			// offscreen render targets
			D3D11_TEXTURE2D_DESC desc{ 0 };
			desc.Format = pixelFormat;
			desc.Width = logW;
			desc.Height = logH;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc = sampleDesc;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
			desc.Usage = D3D11_USAGE_DEFAULT;

			result = device->CreateTexture2D(&desc, nullptr, &target.backBuffer);
			if (FAILED(result))
			{
				return false;
			}
			target.backBuffer->QueryInterface(__uuidof(IDXGIResource1), (void **)&target.backBuffResource);
			target.backBuffResource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, &target.backBuffSharedHandle);
			target.backBuffer->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&target.backBuffMutex);

			// Acquire key 0, immediately release key 0.
			// This puts the texture in a known state (Key 0) ready for Avalonia to read.
			target.backBuffMutex->AcquireSync(0, 100);
			target.backBuffMutex->ReleaseSync(0);
			target.backBuffMutex->Release();
		}
	}
	adapterOutput->Release();
	adapter->Release();
	factory->Release();

	return true;
}

d3d11rs::D3D11RenderSystem::D3D11RenderSystem(uint32_t width, uint32_t height, uint32_t logW, uint32_t logH, Services &services) : System(services)
{
	this->width = width;
	this->height = height;
	this->logW = logW;
	this->logH = logH;
}

d3d11rs::D3D11RenderSystem::~D3D11RenderSystem()
{
	shutdown();
}

void d3d11rs::D3D11RenderSystem::shutdown()
{
	if (textureSharing)
	{
		for (auto &target : renderTargets)
		{
			target.backBuffer->Release();
		}
	}
	if (inputLayout)
	{
		inputLayout->Release();
	}

	if (rasterizerState)
	{
		rasterizerState->Release();
	}

	if (depthStencilView)
	{
		depthStencilView->Release();
	}

	if (depthStencilState)
	{
		depthStencilState->Release();
	}

	if (depthStencilBuffer)
	{
		depthStencilBuffer->Release();
	}

	if (renderTargetView)
	{
		renderTargetView->Release();
	}

	if (immediateCtx)
	{
		immediateCtx->Release();
	}

	if (device)
	{
		device->Release();
	}

	if (swapChain)
	{
		swapChain->SetFullscreenState(false, nullptr);
		swapChain->Release();
	}
}

void d3d11rs::D3D11RenderSystem::update(Node &node)
{
}
