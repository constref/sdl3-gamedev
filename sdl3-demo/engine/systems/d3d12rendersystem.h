#pragma once

#include <systems/system.h>
#include <components/spritecomponent.h>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

struct SDL_Window;

namespace d3d12rs
{

struct FrameResources
{
	Microsoft::WRL::ComPtr<ID3D12Resource> backbuffer;
	uint64_t fenceValue = 0;
};

class D3D12RenderSystem : public System<FrameStage::Render, SpriteComponent>
{
	constexpr static inline uint16_t MaxFrames = 3;
	HWND m_hWnd = NULL;
	int m_width, m_height, m_logW, m_logH;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapchain;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	size_t m_RTVDescriptorSize = 0;
	uint16_t m_backBufferIndex = 0;

	FrameResources m_frameResources[MaxFrames];

	// sync related
	UINT m_syncInterval = 1;
	bool m_allowTearing = false;
	uint16_t m_frameResIndex = 0;
	uint64_t m_frameIndex = 0;
	uint64_t m_fenceValue = 0;
	HANDLE m_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

public:
	D3D12RenderSystem(Services &services, SDL_Window *window, int width, int height, int logW, int logH);
	~D3D12RenderSystem();

	bool initialize();
	void shutdown();

	void beginFrame();
	void endFrame();
	void update(Node &node) override;
	void updateTextures();
	bool createSwapchain();
};

}
