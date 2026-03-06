#pragma once

#include <systems/system.h>
#include <components/meshcomponent.h>

#include <wrl.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <rendering/mesh.h>

#include <span>

struct SDL_Window;

namespace d3d12rs
{
constexpr static uint16_t FramesInFlight = 3;
constexpr static uint16_t RenderTargetCount = 2;
constexpr static uint16_t MaxCopyOps = 32;
constexpr static uint32_t MaxVertCount = 5000;
constexpr static size_t StagingBuffSize = 1024 * 1024 * 128;
constexpr static uint16_t AlignmentVertexIndex = 4;

using Microsoft::WRL::ComPtr;


struct GPUSubMesh
{
	size_t vertexStart = 0;
	size_t vertexCount = 0;
	size_t indexStart = 0;
	size_t indexCount = 0;
};

struct GPUMesh
{
	std::vector<GPUSubMesh> subMeshes;
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexView{};
	D3D12_INDEX_BUFFER_VIEW indexView{};
};

struct CopyOperation
{
	size_t stagingOffset = 0;
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
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldViewProj;
	DirectX::XMFLOAT4X4 invTransWorld;
};

struct RenderObject
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldViewProj;
	DirectX::XMFLOAT4X4 invTransWorld;
};

class D3D12RenderSystem : public System<FrameStage::Render, MeshComponent>
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
	size_t m_stagingOffset = 0;

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
	std::vector<GPUMesh> m_gpuMeshes;

	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12Resource> m_cbBuffPerObj;
	void *m_cbPerObjPtr = nullptr;
	ComPtr<ID3DBlob> m_vsBytecode = nullptr;
	ComPtr<ID3DBlob> m_psBytecode = nullptr;
	ComPtr<ID3D12PipelineState> m_pso;

	// render objects
	ComPtr<ID3D12DescriptorHeap> m_objHeap;
	ComPtr<ID3D12Resource> m_objBuffer;

public:
	D3D12RenderSystem(Services &services, SDL_Window *window, int width, int height, int logW, int logH);
	~D3D12RenderSystem();

	bool initialize();
	GPUMeshHandle loadMesh(const Mesh &mesh);
	const GPUMesh &getMesh(GPUMeshHandle handle);
	SubMesh loadGeometry(const std::span<Vertex> &vertices, const std::span<uint16_t> &indices);
	void loadAssets();
	void shutdown();
	ComPtr<ID3D12PipelineState> createPipelineStateObject();
	ComPtr<ID3DBlob> compileShader(const std::string &file, const std::string &entryPoint, const std::string &target);
	bool createShaders(const std::string &shaderName);

	void beginFrame() override;
	void endFrame() override;
	void update(Node &node) override;
	void updateTextures();
	bool createSwapchain();
	void flushGPU();
	uint32_t stageData(void *srcPtr, size_t byteSize, size_t alignment);
	void scheduleGPUCopy(size_t stagingOffset, size_t dataSize, ComPtr<ID3D12Resource> dstBuffer, D3D12_RESOURCE_STATES dstStateBefore, D3D12_RESOURCE_STATES dstStateAfter);
};

}
