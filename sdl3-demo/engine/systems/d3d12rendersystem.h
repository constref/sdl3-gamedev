#pragma once

#include <systems/system.h>
#include <components/spritecomponent.h>

#include <wrl.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

struct SDL_Window;

namespace d3d12rs
{

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

struct SubMesh
{
	size_t vertexStart = 0;
	size_t vertexCount = 0;
	size_t indexStart = 0;
	size_t indexCount = 0;
};

struct Mesh
{
	std::vector<SubMesh> subMeshes;
};

struct FrameResources
{
	Microsoft::WRL::ComPtr<ID3D12Resource> backbuffer;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	uint64_t fenceValue = 0;
};

class D3D12RenderSystem : public System<FrameStage::Render, SpriteComponent>
{
	constexpr static inline uint16_t MaxFrames = 3;
	constexpr static inline uint32_t MaxVertCount = 5000;
	HWND m_hWnd = NULL;
	int m_width, m_height, m_logW, m_logH;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapchain;
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
