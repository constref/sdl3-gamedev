#pragma once

#include <systems/system.h>
#include <components/meshcomponent.h>

#include <wrl.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <rendering/mesh.h>
#include <rendering/renderer.h>

#include <d3d11on12.h>

#include <span>


struct SDL_Window;

namespace d3d12rs
{
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

struct DescriptorSizes
{
	size_t RTV = 0;
	size_t DSV = 0;
	size_t CBV = 0;
};

struct ConstsPerFrame
{
	DirectX::XMFLOAT4 camPosition;
	DirectX::XMFLOAT4X4 viewProj;
	int numDirectionalLights;
	int numPointLights;
};

struct RenderObject
{
	uint32_t baseMatrixIndex = 0;
};

struct ObjectMatrices
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldViewProj;
	DirectX::XMFLOAT4X4 invTransWorld;
	
	constexpr static uint16_t matrixCount() { return sizeof(ObjectMatrices) / sizeof(DirectX::XMFLOAT4X4); };
};

struct DrawOperation
{
	uint32_t roIndex;
	GPUMeshHandle meshHandle;
};

struct FrameResources
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	uint16_t renderTargetIndex = 0;
	uint64_t fenceValue = 0;
	std::vector<DrawOperation> drawOperations;
};

class D3D12RenderSystem : public System<FrameStage::Render, MeshComponent>, public Renderer
{
	constexpr static inline DXGI_FORMAT SwapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr static inline DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;
	HWND m_hWnd = NULL;
	int m_width, m_height, m_logW, m_logH;

	std::vector<ComPtr<ID3D12Resource>> m_backBuffers;
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
	// internal render targets
	ComPtr<ID3D12Heap> renderTargetHeap;
	std::vector<ComPtr<ID3D12Resource>> m_renderTargetTextures;
	std::vector<HANDLE> m_ntHandles;
	std::vector<ComPtr<IDXGIKeyedMutex>> m_rtKeyedMutexes;
	
	// D3D11 Interop
	std::vector<ComPtr<ID3D11Texture2D>> m_d3d11Targets;
	ComPtr<ID3D11Device> m_device11;
	ComPtr<ID3D11On12Device> m_device11on12;
	ComPtr<ID3D11DeviceContext> m_deviceContext11;

	// staging buffer
	ComPtr<ID3D12Resource> m_stagingBuffer;
	void *m_stagingBasePtr = nullptr;
	void *m_stagingPtr = nullptr;
	size_t m_stagingOffset = 0;

	ComPtr<ID3D12Resource> m_assetStagingBuffer;
	void *m_assetStagingPtr = nullptr;
	size_t m_assetStagingOffset = 0;
	ComPtr<ID3D12CommandAllocator> m_assetCmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_assetCmdList;

	std::vector<CopyOperation> m_copyOperations;
	std::vector<FrameResources> m_frameResources;

	// sync related
	UINT m_syncInterval = 1;
	bool m_allowTearing = false;
	uint16_t m_frameResIndex = 0;
	uint64_t m_frameIndex = 0;
	uint64_t m_fenceValue = 0;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;

	// assets
	std::vector<GPUMesh> m_gpuMeshes;
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	ComPtr<ID3D12Resource> m_cbBuffPerFrame;
	void *m_cbPerFramePtr = nullptr;
	ComPtr<ID3DBlob> m_vsBytecode = nullptr;
	ComPtr<ID3DBlob> m_psBytecode = nullptr;
	ComPtr<ID3D12PipelineState> m_pso;
	
	// camera related
	DirectX::XMFLOAT4 m_camPosition;
	DirectX::XMFLOAT4 m_camDirection;
	DirectX::XMMATRIX m_projMatrix;
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_viewProjMatrix;

	// render objects
	constexpr static uint32_t CBVCount = 1;
	constexpr static uint32_t SRVCount = 2;
	constexpr static uint32_t DescriptorsPerFrame = CBVCount + SRVCount;
	constexpr static size_t ROFrameSize = sizeof(RenderObject) * World::capacity();
	constexpr static size_t ROPerFrame = World::capacity();
	constexpr static size_t MatrixFrameSize = sizeof(ObjectMatrices) * World::capacity();
	constexpr static size_t MatricesPerFrame = ObjectMatrices::matrixCount() * World::capacity();
	uint32_t m_nextROIndex = 0;
	uint32_t m_nextMatrixIndex = 0;
	ComPtr<ID3D12Resource> m_matrixBuffer;
	D3D12_RESOURCE_STATES m_matrixBuffState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	ComPtr<ID3D12Resource> m_objBuffer;
	D3D12_RESOURCE_STATES m_objBufferState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

public:
	D3D12RenderSystem(Services &services, SDL_Window *window, int width, int height, int logW, int logH);
	~D3D12RenderSystem() override;

	bool initialize();
	GPUMeshHandle loadMesh(const Mesh &mesh);
	const GPUMesh &getMesh(GPUMeshHandle handle);
	void loadAssets();
	void releaseAssets() override;
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
	uint32_t stageData(const void *srcPtr, size_t byteSize, size_t alignment);
	void scheduleAssetCopy(size_t stagingOffset, size_t dataSize, ComPtr<ID3D12Resource> dstBuffer, D3D12_RESOURCE_STATES dstStateBefore, D3D12_RESOURCE_STATES dstStateAfter);
	void executeAssetCopyOps();
	
	void setViewMatrix(const DirectX::XMMATRIX &viewMatrix);
	void setCamPosition(float x, float y, float z);
	void setCamDirection(float x, float y, float z);
	std::vector<uint64_t> getSharedTextureHandles(int editorPID) const override;
	void updateGPUTextures() override;

};

}
