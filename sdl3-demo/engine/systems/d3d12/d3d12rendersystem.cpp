#include "d3d12rendersystem.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <logger.h>
#include <SDL3/SDL_system.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <d3d11on12.h>
#include "util.h"

using namespace DirectX;

namespace d3d12rs
{
    constexpr static uint16_t FramesInFlight = 1;
    constexpr static uint16_t RenderTargetCount = 2;
    constexpr static uint16_t MaxCopyOps = 32;
    constexpr static uint32_t MaxVertCount = 5000;
    constexpr static size_t StagingBuffSize = 1024 * 1024 * 32;
    constexpr static uint16_t AlignmentVertexIndex = 4;
}

using namespace Microsoft::WRL;
using namespace d3d12rs;

D3D12RenderSystem::D3D12RenderSystem(Services& services, SDL_Window* window, int width, int height, int logW,
                                     int logH) : System(services)
{
    m_width = width;
    m_height = height;
    m_logW = logW;
    m_logH = logH;
    m_fenceEvent = NULL;
    m_fenceValue = FramesInFlight;
    m_frameResources.resize(FramesInFlight);
    m_camPosition = XMFLOAT4(0, 0, 0, 1);
    m_backBuffers.resize(RenderTargetCount);
    if (Config::IsStandaloneMode())
    {
        m_hWnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER,
                                              NULL);
        assert(m_hWnd && "Unable to acquire HWND for provided window");
    }

    // Logger::logHandler = [](const std::string &message)
    // {
    // 	OutputDebugStringA(std::format("{}\n", message).c_str());
    // };
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

    UINT factoryFlags = Config::DebugSelect(DXGI_CREATE_FACTORY_DEBUG, 0);
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
    DXCHK(D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)),
          "Error creating the D3D12Device");

    // cache the descriptor sizes
    m_descriptorSizes.CBV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_descriptorSizes.RTV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_descriptorSizes.DSV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            // This warning occurs when using capture frame while graphics debugging.
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
    DXCHK(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)),
          "Couldn't create the command queue");

    if constexpr (Config::IsToolingMode())
    {
        // need the 11on12 device to created keyed-mutex from textures later
        DXCHK(D3D11On12CreateDevice(m_device.Get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
                  reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()), 1, 0,
                  m_device11.GetAddressOf(), m_deviceContext11.GetAddressOf(), nullptr),
              "Unable to create D3D11on12 device for texture-sharing");
        DXCHK(m_device11.As(&m_device11on12), "Unable to acquire 11On12Device");
    }

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
        auto& res = m_frameResources[i];
        DXCHK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&res.commandAllocator)),
              "Couldn't create the command allocator");
        DXCHK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, res.commandAllocator.Get(), nullptr,
                  IID_PPV_ARGS(&res.commandList)),
              "Couldn't create the command list");
        DXCHK(res.commandList->Close(), "Couldn't close command list");
    }

    DXCHK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Unable to create fence");
    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_fenceEvent && "Failed to create fence event");

    createSwapchain();
    m_copyOperations.reserve(MaxCopyOps);

    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // staging buffer for copy operations
    constexpr size_t stagingBufferTotalSize = FramesInFlight * StagingBuffSize;
    auto stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(stagingBufferTotalSize);
    DXCHK(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &stagingDesc,
              D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_stagingBuffer.GetAddressOf())),
          "Unable to create the main staging buffer");

    CD3DX12_RANGE readRange(0, 0);
    DXCHK(m_stagingBuffer->Map(0, &readRange, &m_stagingBasePtr), "Unable to map staging buffer");
    m_stagingPtr = m_stagingBasePtr;

    // descriptors and buffers
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descHeapDesc.NumDescriptors = DescriptorsPerFrame * FramesInFlight;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DXCHK(m_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())),
          "Unable to create CBV descriptor heap");

    // per frame constants buffer
    const size_t cbPerFrameSize = align(sizeof(ConstsPerFrame), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    auto cbPerFrameDesc = CD3DX12_RESOURCE_DESC::Buffer(cbPerFrameSize * FramesInFlight);
    DXCHK(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &cbPerFrameDesc,
              D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_cbBuffPerFrame.GetAddressOf())),
          "Unable to create per-obj CB buffer");
    DXCHK(m_cbBuffPerFrame->Map(0, &readRange, &m_cbPerFramePtr), "Unable to map cb buffer");

    // matrix data buffer
    auto srvMatrixDesc = CD3DX12_RESOURCE_DESC::Buffer(MatrixFrameSize * FramesInFlight);
    DXCHK(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &srvMatrixDesc,
              D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_matrixBuffer.GetAddressOf())),
          "Unable to create matrix buffer");

    // Render objects buffer
    auto srvRenderInfoDesc = CD3DX12_RESOURCE_DESC::Buffer(ROFrameSize * FramesInFlight);
    DXCHK(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &srvRenderInfoDesc,
              D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_objBuffer.GetAddressOf())),
          "Unable to create render-object buffer");

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (uint32_t i = 0; i < FramesInFlight; ++i)
    {
        m_frameResources[i].drawOperations.reserve(World::capacity());

        // per frame cbv
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddr = m_cbBuffPerFrame->GetGPUVirtualAddress();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
        cbvDesc.SizeInBytes = cbPerFrameSize;
        cbvDesc.BufferLocation = gpuAddr + i * cbPerFrameSize;
        m_device->CreateConstantBufferView(&cbvDesc, handle);
        handle.Offset(1, m_descriptorSizes.CBV);

        // matrix srv
        auto matSrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::StructuredBuffer(
            MatricesPerFrame, sizeof(XMFLOAT4X4), i * MatricesPerFrame);
        m_device->CreateShaderResourceView(m_matrixBuffer.Get(), &matSrvDesc, handle);
        handle.Offset(1, m_descriptorSizes.CBV);

        // render object srv
        auto objSrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::StructuredBuffer(
            ROPerFrame, sizeof(RenderObject), i * ROPerFrame);
        m_device->CreateShaderResourceView(m_objBuffer.Get(), &objSrvDesc, handle);
        handle.Offset(1, m_descriptorSizes.CBV);
    }

    // create a root signature
    std::array descTable
    {
        CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1),
        CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0)
    };

    std::array<CD3DX12_ROOT_PARAMETER1, 2> rootParameters{};
    rootParameters[0].InitAsConstants(1, 0);
    rootParameters[1].InitAsDescriptorTable(descTable.size(), descTable.data());

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Init_1_1(rootParameters.size(), rootParameters.data(), 0u, nullptr,
                         D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> error = nullptr;
    DXCHK(D3D12SerializeVersionedRootSignature(&rootSigDesc, serializedRootSig.GetAddressOf(), error.GetAddressOf()),
          "Unable to serialize root signature");
    DXCHK(m_device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(),
              IID_PPV_ARGS(m_rootSig.GetAddressOf())), "Error creating root signature");

    loadAssets();
    return true;
}

uint32_t D3D12RenderSystem::stageData(const void* srcPtr, size_t byteSize, size_t alignment)
{
    size_t alignedSize = align(byteSize, alignment);
    assert(
        m_stagingOffset + alignedSize < StagingBuffSize && "Not enough room in staging buffer for the copy operation");
    memcpy(static_cast<uint8_t*>(m_stagingPtr) + m_stagingOffset, srcPtr, byteSize);
    uint32_t dataOffset = m_stagingOffset;
    m_stagingOffset += alignedSize;

    return dataOffset;
}

void D3D12RenderSystem::scheduleGPUCopy(size_t stagingOffset, size_t dataSize, ComPtr<ID3D12Resource> dstBuffer,
                                        D3D12_RESOURCE_STATES dstStateBefore, D3D12_RESOURCE_STATES dstStateAfter)
{
    assert(m_copyOperations.size() < MaxCopyOps && "Copy operations buffer at capacity");
    m_copyOperations.push_back(CopyOperation{
        .stagingOffset = stagingOffset,
        .byteSize = dataSize,
        .dstBuffer = dstBuffer.Get(),
        .dstStateBefore = dstStateBefore,
        .dstStateAfter = dstStateAfter
    });
}

void D3D12RenderSystem::setViewMatrix(const DirectX::XMMATRIX& viewMatrix)
{
    m_viewMatrix = viewMatrix;
}

void D3D12RenderSystem::setCamPosition(float x, float y, float z)
{
    m_camPosition = XMFLOAT4(x, y, z, 1.0f);
}

void D3D12RenderSystem::setCamDirection(float x, float y, float z)
{
    m_camDirection = XMFLOAT4(x, y, z, 0.0f);
}

std::vector<uint64_t> D3D12RenderSystem::getSharedTextureHandles(int editorPID)
{
    assert(editorPID != 0 && "Tooling PID cannot be 0");
    HANDLE editorProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, editorPID);

    if (editorProcess == nullptr)
    {
        auto err = GetLastError();
    }

    std::vector<uint64_t> sharedHandles;
    sharedHandles.reserve(RenderTargetCount);
    for (uint16_t i = 0; i < RenderTargetCount; ++i)
    {
        HANDLE duplicatedHandle = nullptr;
        BOOL success = DuplicateHandle(
            GetCurrentProcess(),
            m_ntHandles[i],
            editorProcess,
            &duplicatedHandle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);
        assert(success && "Unable to duplicate HANDLE");
        sharedHandles.push_back(reinterpret_cast<uint64_t>(duplicatedHandle));
    }
    return sharedHandles;
}

GPUMeshHandle D3D12RenderSystem::loadMesh(const Mesh& mesh)
{
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    //copyBuffer(std::span<uint8_t>(reinterpret_cast<uint8_t *>(vertices.data()), vertsSize), m_boxVerts.Get(),
    //	D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    //const size_t idxsSize = indices.size_bytes();
    //auto resDescI = CD3DX12_RESOURCE_DESC::Buffer(idxsSize);
    //auto buffStateI = D3D12_RESOURCE_STATE_COMMON;
    //m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &resDescI, buffStateI, nullptr, IID_PPV_ARGS(m_boxIndices.GetAddressOf()));
    //copyBuffer(std::span<uint8_t>(reinterpret_cast<uint8_t *>(indices.data()), idxsSize), m_boxIndices.Get(),
    //	D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

    //ibv = D3D12_INDEX_BUFFER_VIEW{};
    //ibv.BufferLocation = m_boxIndices->GetGPUVirtualAddress();
    //ibv.SizeInBytes = idxsSize;
    //ibv.Format = DXGI_FORMAT_R32_UINT;


    // pack verts for all submeshes into staging to prepare for GPU copy
    GPUMesh gpuMesh;
    gpuMesh.subMeshes.resize(mesh.subMeshes().size());

    m_stagingOffset = align(m_stagingOffset, AlignmentVertexIndex);
    const size_t vbStagingStart = m_stagingOffset;
    size_t vertTotalSize = 0;
    uint16_t vertexStart = 0;
    for (int i = 0; i < mesh.subMeshes().size(); ++i)
    {
        auto& sm = mesh.subMeshes()[i];
        gpuMesh.subMeshes[i].vertexStart = vertexStart;
        gpuMesh.subMeshes[i].vertexCount = sm.vertices.size();

        const size_t vertDataSize = sm.vertices.size() * sm.vertexElementSize();
        memcpy(static_cast<uint8_t*>(m_stagingPtr) + m_stagingOffset, sm.vertices.data(), vertDataSize);
        m_stagingOffset += vertDataSize;
        vertTotalSize += vertDataSize;
        vertexStart += sm.vertices.size();
    }

    // pack indices for all submeshes into staging to prepare for GPU copy
    m_stagingOffset = align(m_stagingOffset, AlignmentVertexIndex);
    const size_t ibStagingStart = m_stagingOffset;
    size_t indexTotalSize = 0;
    uint16_t indexStart = 0;
    for (int i = 0; i < mesh.subMeshes().size(); ++i)
    {
        auto& sm = mesh.subMeshes()[i];
        gpuMesh.subMeshes[i].indexStart = indexStart;
        gpuMesh.subMeshes[i].indexCount = sm.indices.size();

        const size_t indexDataSize = sm.indices.size() * sm.indexElementSize();
        memcpy(static_cast<uint8_t*>(m_stagingPtr) + m_stagingOffset, sm.indices.data(), indexDataSize);
        m_stagingOffset += indexDataSize;
        indexTotalSize += indexDataSize;
        indexStart += sm.indices.size();
    }

    // create vb and ib and views
    auto resDescV = CD3DX12_RESOURCE_DESC::Buffer(vertTotalSize);
    auto buffStateV = D3D12_RESOURCE_STATE_COMMON;
    m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &resDescV, buffStateV, nullptr,
                                      IID_PPV_ARGS(gpuMesh.vertexBuffer.GetAddressOf()));
    scheduleGPUCopy(vbStagingStart, vertTotalSize, gpuMesh.vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    gpuMesh.vertexView = D3D12_VERTEX_BUFFER_VIEW{};
    gpuMesh.vertexView.BufferLocation = gpuMesh.vertexBuffer->GetGPUVirtualAddress();
    gpuMesh.vertexView.SizeInBytes = vertTotalSize;
    gpuMesh.vertexView.StrideInBytes = sizeof(Vertex);

    auto resDescI = CD3DX12_RESOURCE_DESC::Buffer(indexTotalSize);
    auto buffStateI = D3D12_RESOURCE_STATE_COMMON;
    m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &resDescI, buffStateI, nullptr,
                                      IID_PPV_ARGS(gpuMesh.indexBuffer.GetAddressOf()));
    scheduleGPUCopy(ibStagingStart, indexTotalSize, gpuMesh.indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                    D3D12_RESOURCE_STATE_INDEX_BUFFER);

    gpuMesh.indexView = D3D12_INDEX_BUFFER_VIEW{};
    gpuMesh.indexView.BufferLocation = gpuMesh.indexBuffer->GetGPUVirtualAddress();
    gpuMesh.indexView.SizeInBytes = indexTotalSize;
    gpuMesh.indexView.Format = (indexTotalSize / indexStart == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    GPUMeshHandle newHandle = m_gpuMeshes.size();
    m_gpuMeshes.push_back(std::move(gpuMesh));
    return newHandle;
}

const GPUMesh& D3D12RenderSystem::getMesh(GPUMeshHandle handle)
{
    return m_gpuMeshes[handle];
}

void D3D12RenderSystem::loadAssets()
{
    bool isSuccess = createShaders("basic");
    assert(isSuccess && "Shader compilation error(s)");

    m_pso = createPipelineStateObject();
}

void D3D12RenderSystem::shutdown()
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
    if (m_cbBuffPerFrame)
    {
        m_cbBuffPerFrame->Unmap(0, nullptr);
    }
    CloseHandle(m_fenceEvent);
}

ComPtr<ID3D12PipelineState> D3D12RenderSystem::createPipelineStateObject()
{
    std::array vertexInputs{
        D3D12_INPUT_ELEMENT_DESC{
            .SemanticName = "POSITION", .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0,
            .AlignedByteOffset = offsetof(Vertex, position)
        },
        D3D12_INPUT_ELEMENT_DESC{
            .SemanticName = "NORMAL", .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0,
            .AlignedByteOffset = offsetof(Vertex, normal)
        },
        D3D12_INPUT_ELEMENT_DESC{
            .SemanticName = "COLOR", .Format = DXGI_FORMAT_R32G32B32A32_FLOAT, .InputSlot = 0,
            .AlignedByteOffset = offsetof(Vertex, color)
        },
        D3D12_INPUT_ELEMENT_DESC{
            .SemanticName = "TEXCOORD", .Format = DXGI_FORMAT_R32G32_FLOAT, .InputSlot = 0,
            .AlignedByteOffset = offsetof(Vertex, uv)
        }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = m_rootSig.Get();
    psoDesc.InputLayout = {.pInputElementDescs = vertexInputs.data(), .NumElements = vertexInputs.size()};
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsBytecode.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psBytecode.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.SampleDesc = {1, 0};
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = RenderTargetCount;
    psoDesc.RTVFormats[0] = SwapchainFormat;
    psoDesc.DSVFormat = DepthStencilFormat;

    ComPtr<ID3D12PipelineState> pso = nullptr;
    if (FAILED(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf()))))
    {
        Logger::error(this, "Error creating Pipeline State Object");
        return nullptr;
    }

    return pso;
}

ComPtr<ID3DBlob> D3D12RenderSystem::compileShader(const std::string& file, const std::string& entryPoint,
                                                  const std::string& target)
{
    const std::wstring wFile = std::wstring(file.begin(), file.end());
    constexpr UINT compileFlags = Config::DebugSelect(D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0);

    ComPtr<ID3DBlob> bytecode = nullptr;
    if (std::filesystem::exists(file))
    {
        Logger::info(this, std::format("Compiling {} : {}() : {}", file, entryPoint, target));
        ComPtr<ID3DBlob> errors = nullptr;
        HRESULT hr = D3DCompileFromFile(wFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(),
                                        target.c_str(),
                                        compileFlags, 0, bytecode.GetAddressOf(), errors.GetAddressOf());
        if (FAILED(hr))
        {
            Logger::error(this, "Error in shader compilation");
            if (errors)
            {
                Logger::error(this, reinterpret_cast<char*>(errors->GetBufferPointer()));
                return nullptr;
            }
        }
    }
    else
    {
        Logger::error(this, std::format("Shader file {} can't be found", file));
        return nullptr;
    }
    return bytecode;
}

bool D3D12RenderSystem::createShaders(const std::string& shaderName)
{
    const std::string prefix = "sdl3-demo/engine/shaders/hlsl";
    const std::string file = std::format("{}/{}.hlsl", prefix, shaderName);
    m_vsBytecode = compileShader(file, "VSMain", "vs_5_1");
    m_psBytecode = compileShader(file, "PSMain", "ps_5_1");
    return true;
}

void D3D12RenderSystem::beginFrame()
{
    m_frameResIndex = m_frameIndex % FramesInFlight;
    auto& res = m_frameResources[m_frameResIndex];

    // ensure frame resources are available
    if (m_fence->GetCompletedValue() < res.fenceValue)
    {
        if (FAILED(m_fence->SetEventOnCompletion(res.fenceValue, m_fenceEvent)))
        {
            Logger::error(this, "Unable to set fence completion event");
            return;
        }
        ::WaitForSingleObject(m_fenceEvent, UINT_MAX);
    }

    // initialize frame data
    m_nextROIndex = 0;
    m_nextMatrixIndex = 0;
    m_stagingPtr = static_cast<uint8_t*>(m_stagingBasePtr) + m_frameResIndex * StagingBuffSize;
    res.drawOperations.clear();
    if constexpr (Config::IsStandaloneMode())
    {
        res.renderTargetIndex = m_swapchain->GetCurrentBackBufferIndex(); 
    }
    else
    {
        res.renderTargetIndex = m_frameIndex % RenderTargetCount;
    }
    
    res.commandAllocator->Reset();
    res.commandList->Reset(res.commandAllocator.Get(), m_pso.Get());

    // process pending copy operations
    if (m_copyOperations.size())
    {
        std::vector<D3D12_RESOURCE_BARRIER> barriers(MaxCopyOps);
        barriers.clear();
        for (int i = 0; i < m_copyOperations.size(); ++i)
        {
            auto& copyOP = m_copyOperations[i];
            res.commandList->CopyBufferRegion(copyOP.dstBuffer.Get(), 0, m_stagingBuffer.Get(), copyOP.stagingOffset,
                                              copyOP.byteSize);

            if (copyOP.dstStateAfter != D3D12_RESOURCE_STATE_COMMON)
            {
                barriers.push_back(
                    CD3DX12_RESOURCE_BARRIER::Transition(copyOP.dstBuffer.Get(), copyOP.dstStateBefore,
                                                         copyOP.dstStateAfter));
            }
        }
        res.commandList->ResourceBarrier(barriers.size(), barriers.data());
        m_copyOperations.clear();
    }

    // transition the matrix and RO buffers to copy-dest
    std::array barriersPreCopy
    {
        CD3DX12_RESOURCE_BARRIER::Transition(m_matrixBuffer.Get(), m_matrixBuffState, D3D12_RESOURCE_STATE_COPY_DEST),
        CD3DX12_RESOURCE_BARRIER::Transition(m_objBuffer.Get(), m_objBufferState, D3D12_RESOURCE_STATE_COPY_DEST)
    };
    res.commandList->ResourceBarrier(barriersPreCopy.size(), barriersPreCopy.data());

    // view and projection calculations
    XMVECTOR camPosition = XMLoadFloat4(&m_camPosition);
    XMVECTOR camDirection = XMLoadFloat4(&m_camDirection);
    XMVECTOR camUp = XMVectorSet(0, 1, 0, 0);
    m_viewMatrix = XMMatrixLookAtLH(camPosition, camPosition + camDirection, camUp);
    const float aspectRatio = m_width / static_cast<float>(m_height);
    m_projMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
    m_viewProjMatrix = m_viewMatrix * m_projMatrix;

    // copy the frame consts into the cbv
    ConstsPerFrame cbPerFrame{};
    XMStoreFloat4x4(&cbPerFrame.viewProj, m_viewProjMatrix);
    const size_t perFrameAlignedSize = align(sizeof(cbPerFrame), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    memcpy(static_cast<uint8_t*>(m_cbPerFramePtr) + perFrameAlignedSize * m_frameResIndex, &cbPerFrame,
           sizeof(cbPerFrame));
}

void D3D12RenderSystem::endFrame()
{
    auto& res = m_frameResources[m_frameResIndex];

    // transition the buffers to SRV compatible
    auto endState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    std::array barriersPostCopy
    {
        CD3DX12_RESOURCE_BARRIER::Transition(m_matrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, endState),
        CD3DX12_RESOURCE_BARRIER::Transition(m_objBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, endState)
    };
    res.commandList->ResourceBarrier(barriersPostCopy.size(), barriersPostCopy.data());

    m_matrixBuffState = endState;
    m_objBufferState = endState;

    // bind root signature
    static std::array descriptorHeaps{m_descriptorHeap.Get()};
    CD3DX12_GPU_DESCRIPTOR_HANDLE descHandle(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
                                             m_frameResIndex * DescriptorsPerFrame, m_descriptorSizes.CBV);

    res.commandList->SetGraphicsRootSignature(m_rootSig.Get());
    res.commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());
    res.commandList->SetGraphicsRootDescriptorTable(1, descHandle);

    // viewport and scissor setup
    static D3D12_VIEWPORT viewport{
        .TopLeftX = 0, .TopLeftY = 0, .Width = static_cast<float>(m_width), .Height = static_cast<float>(m_height),
        .MinDepth = 0, .MaxDepth = 1
    };
    res.commandList->RSSetViewports(1, &viewport);
    static D3D12_RECT scissorRect{.left = 0, .top = 0, .right = m_width, .bottom = m_height};
    res.commandList->RSSetScissorRects(1, &scissorRect);

    ComPtr<ID3D12Resource> renderTarget = Config::ExecSelect(m_backBuffers[res.renderTargetIndex],
                                                             m_renderTargetTextures[res.renderTargetIndex]);
    D3D12_RESOURCE_STATES rtStatePostDraw = Config::ExecSelect(D3D12_RESOURCE_STATE_PRESENT,
                                                               D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto rtBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), rtStatePostDraw,
                                                          D3D12_RESOURCE_STATE_RENDER_TARGET);
    res.commandList->ResourceBarrier(1, &rtBarrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                            res.renderTargetIndex, m_descriptorSizes.RTV);

    FLOAT clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    res.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    FLOAT dsvClear[] = {0.0f, 0.0f, 0.0f, 1.0f};
    res.commandList->ClearDepthStencilView(m_dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0, 0, 0,
                                           nullptr);
    res.commandList->OMSetRenderTargets(1, &rtvHandle, false, &m_dsvHandle);
    res.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ensure we can acquire the keyed-mutex lock
    if constexpr (Config::IsToolingMode())
    {
        HRESULT hr = m_rtKeyedMutexes[res.renderTargetIndex]->AcquireSync(0, 1000);
        if (hr == WAIT_TIMEOUT)
        {
            // do nothing if we can't acquire a lock
            return;
        }
    }

    // draw all objects
    for (const DrawOperation& drawOp : res.drawOperations)
    {
        const GPUMesh& mesh = getMesh(drawOp.meshHandle);
        for (const GPUSubMesh& subMesh : mesh.subMeshes)
        {
            res.commandList->SetGraphicsRoot32BitConstant(0, drawOp.roIndex, 0);
            res.commandList->IASetVertexBuffers(0, 1, &mesh.vertexView);
            res.commandList->IASetIndexBuffer(&mesh.indexView);
            res.commandList->DrawIndexedInstanced(subMesh.indexCount, 1, 0, subMesh.vertexStart, 0);
        }
    }

    D3D12_RESOURCE_STATES stateBeforePostDraw = Config::ExecSelect(D3D12_RESOURCE_STATE_PRESENT,
                                                                   D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                               rtStatePostDraw);
    res.commandList->ResourceBarrier(1, &presentBarrier);

    if constexpr (Config::IsToolingMode())
    {
        m_rtKeyedMutexes[res.renderTargetIndex]->ReleaseSync(1);
    }
    res.commandList->Close();

    std::array<ID3D12CommandList*, 1> commandLists{res.commandList.Get()};
    m_commandQueue->ExecuteCommandLists(commandLists.size(), commandLists.data());

    if constexpr (Config::IsStandaloneMode())
    {
        UINT presentFlags = m_allowTearing && m_syncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;
        m_swapchain->Present(1, presentFlags);
    }
    
    res.fenceValue = ++m_fenceValue;
    m_commandQueue->Signal(m_fence.Get(), res.fenceValue);
    m_frameIndex++;
    m_stagingOffset = 0;
}

void D3D12RenderSystem::update(Node& node)
{
    auto& res = m_frameResources[m_frameResIndex];
    const uint32_t roIndex = m_nextROIndex++;

    RenderObject ro
    {
        .baseMatrixIndex = m_nextMatrixIndex
    };

    // calculate object transformation matrices
    XMMATRIX rotX = XMMatrixRotationX(node.getRotation().x);
    XMMATRIX rotY = XMMatrixRotationY(node.getRotation().y);
    XMMATRIX rotZ = XMMatrixRotationZ(node.getRotation().z);
    XMMATRIX rotation = rotX * rotY * rotZ;
    XMMATRIX world = rotation * XMMatrixTranslation(node.getPosition().x, node.getPosition().y, node.getPosition().z);
    XMMATRIX worldViewProj = world * m_viewProjMatrix;
    XMMATRIX normalMat = world;
    normalMat.r[3] = XMVectorSet(0, 0, 0, 1); // remove translation from normal transform
    XMMATRIX invTransWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, normalMat));

    // update the per-obj data
    ObjectMatrices objMatrices;
    XMStoreFloat4x4(&objMatrices.world, world);
    XMStoreFloat4x4(&objMatrices.worldViewProj, XMMatrixTranspose(worldViewProj));
    XMStoreFloat4x4(&objMatrices.invTransWorld, invTransWorld);
    m_nextMatrixIndex += ObjectMatrices::matrixCount();

    uint32_t stageMatrixOffset = stageData(&objMatrices, sizeof(objMatrices), 4);
    uint32_t stageRObjOffset = stageData(&ro, sizeof(ro), 4);

    res.commandList->CopyBufferRegion(m_matrixBuffer.Get(),
                                      (m_frameResIndex * MatrixFrameSize) + ro.baseMatrixIndex * sizeof(XMFLOAT4X4),
                                      m_stagingBuffer.Get(), stageMatrixOffset, sizeof(objMatrices));
    res.commandList->CopyBufferRegion(m_objBuffer.Get(),
                                      (m_frameResIndex * ROFrameSize) + roIndex * sizeof(RenderObject),
                                      m_stagingBuffer.Get(), stageRObjOffset, sizeof(RenderObject));

    auto [mc] = getRequiredComponents(node);
    res.drawOperations.push_back(DrawOperation{.roIndex = roIndex, .meshHandle = mc->getHandle()});
}

void D3D12RenderSystem::updateTextures()
{
}

bool D3D12RenderSystem::createSwapchain()
{
    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = RenderTargetCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    DXCHK(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVDescriptorHeap)),
          "Unable to create RTV descriptor heap");
    // DSV Heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    DXCHK(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVDescriptorHeap)),
          "Unable to create DSV descriptor heap");

    m_renderTargetTextures.resize(RenderTargetCount);
    if constexpr (Config::IsStandaloneMode())
    {
        // create internal render targets
        constexpr auto rtFlags =
            Config::ExecSelect(
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS);
        auto renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(SwapchainFormat, m_logW, m_logH,
                                                             1, 0, 1, 0, rtFlags);
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);

        D3D12_CLEAR_VALUE clearValue{.Format = SwapchainFormat, .Color = {1, 0, 0, 1}};

        m_renderTargetTextures.resize(RenderTargetCount);
        for (auto& renderTarget : m_renderTargetTextures)
        {
            DXCHK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &renderTargetDesc,
                      D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(renderTarget.GetAddressOf())),
                  "Unable to create internal render target texture");
        }

        Logger::info(this, "Creating and initializing swapchain resources");
        ComPtr<IDXGISwapChain4> dxgiSwapchain;
        ComPtr<IDXGIFactory4> dxgiFactory4;

        UINT factoryFlags = Config::DebugSelect(DXGI_CREATE_FACTORY_DEBUG, 0);
        DXCHK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dxgiFactory4)),
              "Couldn't create DXGIFactory2 during swapchain init");

        DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
        swapchainDesc.Width = m_width;
        swapchainDesc.Height = m_height;
        swapchainDesc.Format = SwapchainFormat;
        swapchainDesc.Stereo = FALSE;
        swapchainDesc.SampleDesc = {1, 0};
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.BufferCount = RenderTargetCount;
        swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchainDesc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapchain;
        DXCHK(dxgiFactory4->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hWnd, &swapchainDesc, nullptr, nullptr, &
                  swapchain), "Failed to create a swapchain for the given HWND");
        DXCHK(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER),
              "Failed to disable full-screen shortcut.");
        DXCHK(swapchain.As(&m_swapchain), "Error getting swapchain");

        // grab the swapchain image buffers
        for (int i = 0; i < RenderTargetCount; ++i)
        {
            DXCHK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(m_backBuffers[i].GetAddressOf())),
                  "Unable to get swapchain back buffer");
        }
    }
    else
    {
        // create D3D11 textures, handles and keyed mutexes, mutecies? muti?
        m_rtKeyedMutexes.resize(RenderTargetCount);
        m_ntHandles.resize(RenderTargetCount);
        m_d3d11Targets.resize(RenderTargetCount);

        for (uint16_t i = 0; i < RenderTargetCount; ++i)
        {
            // offscreen render targets
            D3D11_TEXTURE2D_DESC desc{0};
            desc.Format = SwapchainFormat;
            desc.Width = m_logW;
            desc.Height = m_logH;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.SampleDesc = {.Count = 1, .Quality = 0};
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
            desc.Usage = D3D11_USAGE_DEFAULT;

            // create the texture and handle
            DXCHK(m_device11->CreateTexture2D(&desc, nullptr, m_d3d11Targets[i].GetAddressOf()),
                  "Unable to create D3D11 shared texture2D");
            ComPtr<IDXGIResource1> dxgiResource;
            m_d3d11Targets[i].As(&dxgiResource);
            dxgiResource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
                                             &m_ntHandles[i]);

            // acquire the keyed mutex
            m_d3d11Targets[i].As(&m_rtKeyedMutexes[i]);
            m_rtKeyedMutexes[i]->AcquireSync(0, 100);
            m_rtKeyedMutexes[i]->ReleaseSync(0);

            // acquire D3D12 textures from NT handles
            m_device->OpenSharedHandle(m_ntHandles[i], IID_PPV_ARGS(m_renderTargetTextures[i].GetAddressOf()));
        }
    }
    // create the render target views
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (int i = 0; i < RenderTargetCount; ++i)
    {
        ComPtr<ID3D12Resource> renderTarget = Config::ExecSelect(m_backBuffers[i], m_renderTargetTextures[i]);
        m_device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_descriptorSizes.RTV);
    }

    // create a depth/stencil view
    D3D12_RESOURCE_DESC dsDesc{};
    dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    dsDesc.Alignment = 0;
    dsDesc.Width = m_logW;
    dsDesc.Height = m_logH;
    dsDesc.DepthOrArraySize = 1;
    dsDesc.MipLevels = 1;
    dsDesc.Format = DepthStencilFormat;
    dsDesc.SampleDesc = {1, 0};
    dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE dsClearVal{};
    dsClearVal.Format = DepthStencilFormat;
    dsClearVal.DepthStencil = {1, 0};

    CD3DX12_HEAP_PROPERTIES dsHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    DXCHK(m_device->CreateCommittedResource(&dsHeapProps, D3D12_HEAP_FLAG_NONE, &dsDesc,
              D3D12_RESOURCE_STATE_DEPTH_WRITE, &dsClearVal, IID_PPV_ARGS(m_depthStencil.GetAddressOf())),
          "Unable to create depth resource");

    m_descriptorSizes.DSV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, dsvHandle);
    m_dsvHandle = dsvHandle;

    return true;
}

void D3D12RenderSystem::flushGPU()
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
