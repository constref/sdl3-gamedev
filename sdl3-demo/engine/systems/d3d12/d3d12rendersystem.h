#pragma once

#include <systems/system.h>
#include <components/spritecomponent.h>

#include <wrl.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#include <span>

struct SDL_Window;

namespace d3d12rs
{
constexpr static uint16_t FramesInFlight = 3;
constexpr static uint16_t RenderTargetCount = 2;
constexpr static uint16_t MaxCopyOps = 32;
constexpr static uint32_t MaxVertCount = 5000;
constexpr static size_t StagingBuffSize = 1024 * 1024 * 128;

using Microsoft::WRL::ComPtr;

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
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

struct CopyOperation
{
	uint32_t stagingOffset = 0;
	size_t byteSize = 0;
	ComPtr<ID3D12Resource> dstBuffer;
	D3D12_RESOURCE_STATES dstStateBefore;
	D3D12_RESOURCE_STATES dstStateAfter;
};

struct FrameResources
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	uint16_t renderTargetIndex = 0;
	uint64_t fenceValue = 0;
};

struct DescriptorSizes
{
	size_t RTV = 0;
	size_t DSV = 0;
	size_t CBV = 0;
};

struct ConstsPerObject
{
	DirectX::XMFLOAT4X4 worldViewProj;
};

class D3D12RenderSystem : public System<FrameStage::Render, SpriteComponent>
{
	constexpr static inline DXGI_FORMAT SwapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr static inline DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;
	HWND m_hWnd = NULL;
	int m_width, m_height, m_logW, m_logH;

	std::array<ComPtr<ID3D12Resource>, RenderTargetCount> m_backBuffers;
	ComPtr<IDXGIAdapter4> m_dxgiAdapter;
	ComPtr<ID3D12Device2> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapchain;
	ComPtr<ID3D12Resource> m_depthStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;
	ComPtr<ID3D12RootSignature> m_rootSig;
	ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> m_DSVDescriptorHeap;
	DescriptorSizes m_descriptorSizes;
	uint16_t m_backBufferIndex = 0;

	// staging buffer
	ComPtr<ID3D12Resource> m_stagingBuffer;
	void *m_stagingPtr = nullptr;
	uint32_t m_stagingOffset = 0;

	std::vector<CopyOperation> m_copyOperations;
	FrameResources m_frameResources[FramesInFlight];

	// sync related
	UINT m_syncInterval = 1;
	bool m_allowTearing = false;
	uint16_t m_frameResIndex = 0;
	uint64_t m_frameIndex = 0;
	uint64_t m_fenceValue = FramesInFlight;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;

	// assets
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12Resource> m_boxVerts, m_boxIndices;
	D3D12_VERTEX_BUFFER_VIEW vbv{};
	D3D12_INDEX_BUFFER_VIEW ibv{};
	ComPtr<ID3D12Resource> m_cbBuffPerObj;
	void *m_cbPerObjPtr = nullptr;
	ComPtr<ID3DBlob> m_vsBytecode = nullptr;
	ComPtr<ID3DBlob> m_psBytecode = nullptr;
	ComPtr<ID3D12PipelineState> m_pso;

public:
	D3D12RenderSystem(Services &services, SDL_Window *window, int width, int height, int logW, int logH);
	~D3D12RenderSystem();

	bool initialize();
	void loadAssets();
	void shutdown();
	ComPtr<ID3D12PipelineState> createPipelineStateObject();
	ComPtr<ID3DBlob> compileShader(const std::string &file, const std::string &entryPoint, const std::string &target);
	bool createShaders(const std::string &shaderName);

	void beginFrame();
	void endFrame();
	void update(Node &node) override;
	void updateTextures();
	bool createSwapchain();
	void flushGPU();
	uint32_t stageData(void *srcPtr, size_t byteSize, size_t alignment);
	void copyBuffer(std::span<uint8_t> srcData, ComPtr<ID3D12Resource> dst,
		D3D12_RESOURCE_STATES dstStateBefore, D3D12_RESOURCE_STATES dstStateAfter = D3D12_RESOURCE_STATE_COMMON);
};

}
