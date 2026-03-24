#include "usdprocessor.h"
#include <filesystem>
#include <unordered_map>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <pxr/base/tf/diagnosticMgr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/usd/primRange.h>

#include <logger.h>
#include <tooling/platformutils.h>
#include <pxr/imaging/hd/meshUtil.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/imaging/hd/meshTopologySchema.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/tokens.h>

#include <systems/d3d12/d3d12rendersystem.h>
#include <vulkan/vulkan_core.h>

#include "componentsystems.h"

using namespace pxr;
using namespace DirectX;
using namespace usd;

class UsdLogger : public TfDiagnosticMgr::Delegate
{
public:
    void IssueError(const TfError &err) override
    {
        Logger::error(this, std::format("{}", err.GetErrorCodeAsString()));
    }

    void IssueFatalError(const TfCallContext &context,
                         const std::string &msg) override
    {
        Logger::error(this, std::format("USD Fatal Error: {}", msg));
    }

    void IssueStatus(const TfStatus &status) override
    {
        Logger::info(this, std::format("USD Status: {}", status.GetCommentary()));
    }

    void IssueWarning(const TfWarning &warning) override
    {
        Logger::warn(this, std::format("{}", warning.GetCommentary()));
    }
};

static UsdLogger g_usdLogger;

struct VertexId
{
    uint32_t index = UINT32_MAX;
    GfVec3f normal;
    GfVec2f uv;
    constexpr static float epsilon = 0.001f;

    bool operator==(const VertexId &o) const
    {
        const float nDot = GfDot(normal, o.normal);
        return index == o.index && nDot > 0.999f &&
            isSame(uv[0], o.uv[0]) &&
            isSame(uv[1], o.uv[1]);
    }

    struct Hasher
    {
        size_t operator()(const VertexId &ve) const
        {
            size_t seed = 0;
            auto combine = [&](size_t value)
            {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };
            auto snap = [](float value)
            {
                return static_cast<int>(std::round(value / VertexId::epsilon));
            };

            std::hash<uint32_t> hashUint{};
            std::hash<int> hashInt{};

            combine(hashUint(ve.index));
            combine(hashInt(snap(ve.normal[0])));
            combine(hashInt(snap(ve.normal[1])));
            combine(hashInt(snap(ve.normal[2])));
            combine(hashInt(snap(ve.uv[0])));
            combine(hashInt(snap(ve.uv[1])));

            return seed;
        }
    };

private:
    static bool isSame(float a, float b)
    {
        return std::abs(a - b) < epsilon;
    }
};


static std::unique_ptr<Mesh> processMesh(UsdProcessor* self, UsdGeomMesh mesh)
{
    Logger::info(self, std::format("Processing UsdGeomMesh:{}", mesh.GetPath().GetString()));

    UsdGeomPrimvarsAPI primvarApi(mesh);
    VtArray<GfVec3f> points;
    mesh.GetPointsAttr().Get(&points);
    VtArray<int> faceCounts;
    mesh.GetFaceVertexCountsAttr().Get(&faceCounts);
    VtArray<int> indices;
    mesh.GetFaceVertexIndicesAttr().Get(&indices);

    HdMeshTopology topology(UsdGeomTokens->none, UsdGeomTokens->rightHanded, faceCounts, indices);
    HdMeshUtil meshUtil(&topology, mesh.GetPath());

    VtVec3iArray newIndices;
    VtIntArray primitiveParams;
    meshUtil.ComputeTriangleIndices(&newIndices, &primitiveParams);

    std::vector<Vertex> submeshVerts;
    submeshVerts.reserve(points.size());
    std::vector<uint16_t> submeshIndices;
    submeshIndices.reserve(newIndices.size());

    // vertex normals
    VtArray<GfVec3f> usdNormals;
    mesh.GetNormalsAttr().Get(&usdNormals);
    assert(
        mesh.GetNormalsInterpolation() == UsdGeomTokens->faceVarying &&
        "Only face varying normals are currently supported");

    // triangulate the normals from usd source
    VtValue triNormalsVal;
    HdMeshComputationResult triNormResult = meshUtil.ComputeTriangulatedFaceVaryingPrimvar(
        usdNormals.data(), usdNormals.size(), HdTypeFloatVec3, &triNormalsVal);
    if (triNormResult == HdMeshComputationResult::Error)
    {
        Logger::error(self, "Error triangulating face normals");
    }
    VtVec3fArray triNormals = triNormalsVal.Get<VtArray<GfVec3f>>();
    for (GfVec3f &n : triNormals)
    {
        n.Normalize();
    }

    // get UVs (st)
    UsdGeomPrimvar stPrimvar = primvarApi.GetPrimvar(TfToken("st"));
    VtArray<GfVec2f> stList;
    stPrimvar.ComputeFlattened(&stList);
    VtValue stTriVal;
    HdMeshComputationResult triStResult = meshUtil.ComputeTriangulatedFaceVaryingPrimvar(
        stList.begin(), stList.size(), HdTypeFloatVec2, &stTriVal);
    VtVec2fArray triSts = stTriVal.Get<VtArray<GfVec2f>>();

    const uint32_t indexCount = newIndices.size() * 3;
    assert(
        (triNormals.size() == indexCount && triSts.size() == indexCount) &&
        "Normals / UVs and index counts must match");
    Logger::info(self, "Vertex data triangulated, generating vertex and index data");

    // map that will hold each unique vertex entry during processing
    std::unordered_map<VertexId, uint32_t, VertexId::Hasher> vertexMap;
    submeshVerts.reserve(points.size());
    submeshIndices.reserve(newIndices.size() * 3);

    // generate unique vertex IDs (pos, norm, uv) and store each unique variation
    uint32_t indexPos = 0;
    for (uint32_t i = 0; i < newIndices.size(); ++i)
    {
        const GfVec3i vec3idx = newIndices[i];
        for (short vi = 0; vi < 3; ++vi)
        {
            const uint32_t idx = vec3idx[vi];
            GfVec3f usdNorm = triNormals[indexPos];
            GfVec2f usdSt = triSts[indexPos];

            // generate a vertex ID to check against the hash
            VertexId ve
            {
                .index = idx,
                .normal = usdNorm,
                .uv = usdSt
            };

            auto [itr, isNewVertex] = vertexMap.try_emplace(ve, static_cast<size_t>(submeshVerts.size()));
            if (isNewVertex)
            {
                // vertex wasn't in the map, create a new mesh vert
                Vertex v;
                v.position = XMFLOAT3(points[idx][0], points[idx][1], -points[idx][2]);
                v.normal = XMFLOAT3(ve.normal[0], ve.normal[1], -ve.normal[2]);
                v.uv = XMFLOAT2(ve.uv[0], ve.uv[1]);
                v.color = XMFLOAT4(1, 1, 1, 1);
                submeshVerts.push_back(v);
            }
            submeshIndices.push_back(itr->second);
            indexPos++;
        }
    }
    Logger::info(self, std::format("Submesh generated with {} vertices and {} indices", submeshVerts.size(),
                                   submeshIndices.size()));
    std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>();
    newMesh->addSubmesh(SubMesh(submeshVerts, submeshIndices));
    return std::move(newMesh);
}

static void processPrim(UsdProcessor* self, UsdPrim prim, Node &parent, Services &services,
                        std::unordered_map<std::string, PrimGeo> &meshes)
{
    Logger::info(self, std::format("Processing path: {}", prim.GetPath().GetString()));
    World &world = services.world();
    d3d12rs::D3D12RenderSystem* renderer = services.compSys().getSystemRegistry().getSystem<
        d3d12rs::D3D12RenderSystem>();

    // lambda to process geom mesh and attach component to runtime Node
    auto processGeomMesh = [self, &meshes, &services, renderer](Node &node, UsdGeomMesh meshPrim)
    {
        const std::string path = meshPrim.GetPath().GetString();
        Logger::info(self, std::format("Geomesh path: {}: ", path));

        auto meshItr = meshes.find(path);
        if (meshItr == meshes.end())
        {
            std::unique_ptr<Mesh> cpuMesh = processMesh(self, meshPrim);
            GPUMeshHandle gpuHandle = renderer->loadMesh(*cpuMesh);

            PrimGeo primGeo{
                .mesh = std::move(cpuMesh),
                .gpuHandle = gpuHandle
            };

            auto [itr, added] = meshes.insert({path, std::move(primGeo)});
            meshItr = itr;
        }

        services.compSys().addComponent<MeshComponent>(node, meshItr->second.gpuHandle);
    };

    auto extractTransform = [](UsdPrim prim, Node &node)
    {
        UsdGeomXformable xf(prim);
        bool resetsXformStack = false;
        auto ops = xf.GetOrderedXformOps(&resetsXformStack);
        for (UsdGeomXformOp op : ops)
        {
            if (op.GetOpType() == UsdGeomXformOp::TypeTranslate)
            {
                GfVec3d translation;
                bool got = op.Get<GfVec3d>(&translation);
                node.setPosition(glm::vec3(translation[0], translation[1], -translation[2]));
            }
            else if (op.GetOpType() == UsdGeomXformOp::TypeRotateXYZ)
            {
                if (op.GetPrecision() == UsdGeomXformOp::PrecisionFloat)
                {
                    GfVec3f rotations;
                    bool got = op.Get<GfVec3f>(&rotations);
                    node.setRotation(XMFLOAT3(-DirectX::XMConvertToRadians(rotations[0]),
                                              -DirectX::XMConvertToRadians(rotations[1]),
                                              DirectX::XMConvertToRadians(rotations[2])));
                }
            }
            else if (op.GetOpType() == UsdGeomXformOp::TypeScale)
            {
            }
        }
    };

    auto &typeInfo = prim.GetPrimTypeInfo();
    if (typeInfo.GetTypeName() == UsdGeomTokens->Xform)
    {
        // Xform, create a node with transform
        NodeHandle hNode = world.createNode();
        Node &node = world.getNode(hNode);
        parent.addChild(node);

        extractTransform(prim, node);
        if (prim.IsInstance())
        {
            // refer to prim's prototype
            UsdPrim prototype = prim.GetPrototype();
            for (UsdPrim child : prototype.GetChildren())
            {
                if (child.GetTypeName() == UsdGeomTokens->Mesh)
                {
                    UsdGeomMesh meshPrim(child);
                    processGeomMesh(node, meshPrim);
                }
            }
        }
        else
        {
            // look directly in prim's tree for mesh (not instance)
            for (UsdPrim child : prim.GetChildren())
            {
                if (child.GetTypeName() == UsdGeomTokens->Xform)
                {
                    processPrim(self, child, parent, services, meshes);
                }
                else if (child.GetTypeName() == UsdGeomTokens->Mesh)
                {
                    UsdGeomMesh meshPrim(child);
                    processGeomMesh(node, meshPrim);
                }
            }
        }
    }
    else if (prim.GetTypeName() == UsdGeomTokens->Scope)
    {
        // scopes are only for organization
        if (prim.IsActive())
        {
            // TODO: Ensure no material should be inherited from this Scope prim
            for (UsdPrim child : prim.GetChildren())
            {
                processPrim(self, child, parent, services, meshes);
            }
        }
    }
    else if (prim.GetTypeName() == UsdGeomTokens->Mesh)
    {
        NodeHandle hNode = world.createNode();
        Node &node = world.getNode(hNode);
        parent.addChild(node);

        extractTransform(prim, node);
        UsdGeomMesh meshPrim(prim);
        processGeomMesh(node, meshPrim);
    }
}

class StageProcessor : public TfWeakBase
{
    pxr::UsdStageRefPtr m_stage;
public:
    UsdStageRefPtr stage() const { return m_stage; }
    void setStage(UsdStageRefPtr stage) { m_stage = stage; }
    
    void onObjectsChanged(const UsdNotice::ObjectsChanged &notice)
    {
        for (auto &path : notice.GetResyncedPaths())
        {
            Logger::info(this, std::format("Affected path: {}", path.GetAsString()));
        }
    }
};

UsdProcessor::UsdProcessor()
{
    TfDiagnosticMgr::GetInstance().AddDelegate(&g_usdLogger);
    m_noticeHandler = new StageProcessor;
    TfNotice::Register(TfCreateWeakPtr(m_noticeHandler), &StageProcessor::onObjectsChanged);
    
    // if (!std::filesystem::exists(""))
    // {
    // 	throw std::runtime_error("Unable to find USD file");
    // }
    //
    // auto stage = pxr::UsdStage::Open("");

    /*
    auto range = stage->Traverse();
    for (auto itr = range.begin(); itr != range.end(); ++itr)
    {
        UsdPrim prim = *itr;
        processPrim(this, prim, root, services, m_meshes);
    }
    */
    /*
    UsdPrim rootPrim = stage-e->GetPseudoRoot();
    for (auto child : rootPrim.GetChildren())
    {
        processPrim(this, child, root, services, meshes);
    }
*/
}

UsdProcessor::~UsdProcessor()
{
    delete m_noticeHandler;
}

void UsdProcessor::createStage(const std::string &path)
{
    m_noticeHandler->setStage(pxr::UsdStage::CreateNew(path));
}

void UsdProcessor::openStage(const std::string &usdPath)
{
    auto stage = pxr::UsdStage::Open(usdPath);
    m_noticeHandler->setStage(stage);
}

void UsdProcessor::saveStage()
{
    m_noticeHandler->stage()->Save();
}

void UsdProcessor::addLayer(const std::string &layerPath)
{
    if (std::filesystem::exists(layerPath))
    {
        m_noticeHandler->stage()->GetRootLayer()->GetSubLayerPaths().push_back(layerPath);
    }
    else
    {
        Logger::error(this, "File path does not exist");
    }
}

void UsdProcessor::addPrim(const std::string &path, const std::string &type)
{
    SdfPath primPath(path);
    TfToken primType = UsdGeomTokens->Scope;
    if (type == "Xform")
    {
        primType = UsdGeomTokens->Xform;
    }
    UsdPrim newPrim = m_noticeHandler->stage()->DefinePrim(primPath, primType);
}

void UsdProcessor::bakeStage(Node &root, Services &services)
{
    UsdPrim rootPrim = m_noticeHandler->stage()->GetPseudoRoot();
    for (auto child : rootPrim.GetChildren())
    {
        processPrim(this, child, root, services, m_meshes);
    }
    d3d12rs::D3D12RenderSystem* renderer = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();
    renderer->executeAssetCopyOps();
}