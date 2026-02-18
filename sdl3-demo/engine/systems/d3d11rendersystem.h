#pragma once

#include <vector>
#include <systems/system.h>
#include <components/spritecomponent.h>
#include <d3d11.h>
#include <dxgi1_3.h>

/*
This class is currently only for Vulkan->D3D Interop, but may turn into a full D3D11 backend for the engine
*/

namespace d3d11rs
{

struct RenderTarget
{
	ID3D11Texture2D *backBuffer = nullptr;
	IDXGIResource1 *backBuffResource = nullptr;
	HANDLE backBuffSharedHandle = nullptr;
	IDXGIKeyedMutex *backBuffMutex = nullptr;
};

class D3D11RenderSystem : public System<FrameStage::Render, SpriteComponent>
{
	bool textureSharing = true;
	uint32_t width = 0, height = 0;
	uint32_t logW = 0, logH = 0;

	IDXGISwapChain1 *swapChain = nullptr;
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *immediateCtx = nullptr;
	ID3D11RenderTargetView *renderTargetView = nullptr;
	ID3D11Texture2D *depthStencilBuffer = nullptr;
	ID3D11DepthStencilState *depthStencilState = nullptr;
	ID3D11DepthStencilView *depthStencilView = nullptr;
	ID3D11RasterizerState *rasterizerState = nullptr;
	D3D11_VIEWPORT viewport;
	ID3D11InputLayout *inputLayout = nullptr;

	std::wstring videoCardDesc;
	size_t videoMemMb;

	std::vector<RenderTarget> renderTargets;

public:
	D3D11RenderSystem(uint32_t width, uint32_t height, uint32_t logW, uint32_t logH, Services &services);
	~D3D11RenderSystem();

	// Inherited via System
	bool initialize(uint32_t backBufferCount);
	void shutdown();
	void update(Node &node) override;

	auto &getRenderTargets()
	{
		return renderTargets;
	}
};

};

