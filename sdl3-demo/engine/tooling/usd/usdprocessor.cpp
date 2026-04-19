#include "usdprocessor.h"

#include <filesystem>
#include <functional>

#define NOMINMAX
#include <pxr/base/tf/diagnosticMgr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/meshUtil.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/imaging/hd/meshTopologySchema.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usd/tokens.h>
#include <pxr/usd/usd/notice.h>

#include <systems/d3d12/d3d12rendersystem.h>
#include <vulkan/vulkan_core.h>
#include <logger.h>
#include <tooling/platformutils.h>
#include <tooling/usd/usdstagelistener.h>
#include <componentsystems.h>
#include <rendering/mesh.h>
#include "usdlogger.h"

using namespace pxr;
using namespace DirectX;
using namespace usd;

const SdfPath WorldPath("/World");
const SdfPath LibPath("/Library");
const SdfPath MeshesPath(LibPath.AppendPath(SdfPath("Meshes")));
const SdfPath BrushesPath(LibPath.AppendPath(SdfPath("Brushes")));

namespace QUBED
{
    namespace Keys
    {
		const TfToken Type("qubed:type");
    }
    namespace Values
    {
		const VtValue MeshType("mesh");
		const VtValue BrushType("brush");
    }
}

namespace usd
{

class UsdMembers : public pxr::TfWeakBase
{
    UsdStageRefPtr m_stage;
    SdfLayerRefPtr m_worldLayer;
    UsdLogger m_usdLogger;
    std::shared_ptr<usd::UsdStageListener> m_stageListener;

public:
    UsdMembers(std::shared_ptr<usd::UsdStageListener> stageListener) : m_stageListener(stageListener)
    {
        TfDiagnosticMgr::GetInstance().AddDelegate(&m_usdLogger);
    }

    auto stage() const { return m_stage; }
    void setStage(UsdStageRefPtr stage) { m_stage = stage; }
    auto worldLayer() { return m_worldLayer; }
    void setWorldLayer(SdfLayerRefPtr layer) { m_worldLayer = layer; }
};

}


struct VertexId
{
    uint32_t index = UINT32_MAX;
    GfVec3f normal;
    GfVec2f uv;
    constexpr static float epsilon = 0.001f;

    bool operator==(const VertexId& o) const
    {
        const float nDot = GfDot(normal, o.normal);
        return index == o.index && nDot > 0.999f &&
            isSame(uv[0], o.uv[0]) &&
            isSame(uv[1], o.uv[1]);
    }

    struct Hasher
    {
        size_t operator()(const VertexId& ve) const
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


static std::unique_ptr<Mesh> processMesh(UsdProcessor *self, UsdGeomMesh mesh)
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
    for (GfVec3f& n : triNormals)
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

            auto [itr, isNewVertex] = vertexMap.try_emplace(ve, static_cast<uint32_t>(submeshVerts.size()));
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

static void processPrim(UsdProcessor *self, UsdPrim prim, Node& parent, Services& services,
                        std::unordered_map<std::string, PrimGeo>& meshes)
{
    Logger::info(self, std::format("Processing path: {}", prim.GetPath().GetString()));
    World& world = services.world();
    d3d12rs::D3D12RenderSystem *renderer = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();

    // lambda to process geom mesh and attach component to runtime Node
    auto processGeomMesh = [self, &meshes, &services, renderer](Node& node, UsdGeomMesh meshPrim)
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

    auto extractTransform = [](UsdPrim prim, Node& node)
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

    auto& typeInfo = prim.GetPrimTypeInfo();
    if (typeInfo.GetTypeName() == UsdGeomTokens->Xform)
    {
        // Xform, create a node with transform
        NodeHandle hNode = world.createNode();
        Node& node = world.getNode(hNode);
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
                if (child.GetTypeName() == UsdGeomTokens->Mesh)
                {
                    UsdGeomMesh meshPrim(child);
                    processGeomMesh(node, meshPrim);
                }
                else
                {
                    processPrim(self, child, parent, services, meshes);
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
        Node& node = world.getNode(hNode);
        parent.addChild(node);

        extractTransform(prim, node);
        UsdGeomMesh meshPrim(prim);
        processGeomMesh(node, meshPrim);
    }
}

UsdProcessor::UsdProcessor(std::shared_ptr<usd::UsdStageListener> listener)
{
    m_usdMembers = std::make_unique<UsdMembers>(listener);
    if (listener)
    {
        TfNotice::Register(TfCreateWeakPtr(listener.get()), &UsdStageListener::onObjectsChanged);
    }
}

UsdProcessor::~UsdProcessor()
{
    //TfNotice::Revoke(m_revokeKey);
}

UsdStageRefPtr UsdProcessor::stage()
{
    return m_usdMembers->stage();
}

void UsdProcessor::createStage(const std::string& path)
{
    auto stage = pxr::UsdStage::CreateNew(path);

    // setup /Library structure
    UsdPrim libPrim = stage->DefinePrim(LibPath, UsdGeomTokens->Scope);
    UsdGeomImageable libImg(libPrim);
    libImg.GetVisibilityAttr().Set(UsdGeomTokens->invisible);

    stage->DefinePrim(MeshesPath, UsdGeomTokens->Scope);
    stage->DefinePrim(BrushesPath, UsdGeomTokens->Scope);

    // setup /World
    UsdPrim world = stage->DefinePrim(WorldPath, UsdGeomTokens->Xform);
    stage->SetDefaultPrim(world);

    std::filesystem::path stagePath(path);
    auto worldLayer = SdfLayer::CreateNew("layout.usda");

    stage->GetRootLayer()->GetSubLayerPaths().push_back(worldLayer->GetRealPath());
    stage->GetLayerStack(false).push_back(worldLayer);
    worldLayer->Save();
    stage->Save();

    m_usdMembers->setStage(stage);
    m_usdMembers->setWorldLayer(worldLayer);
}

void UsdProcessor::openStage(const std::string& usdPath)
{
    UsdStageRefPtr stage = pxr::UsdStage::Open(usdPath);
    m_usdMembers->setStage(stage);
}

void UsdProcessor::saveStage() const
{
    m_usdMembers->stage()->Save();
}

void UsdProcessor::addLayer(const std::string& layerPath)
{
    if (std::filesystem::exists(layerPath))
    {
        m_usdMembers->stage()->GetRootLayer()->GetSubLayerPaths().push_back(layerPath);
    }
    else
    {
        Logger::error(this, "File path does not exist");
    }
}

void UsdProcessor::addPrim(const std::string& path, const std::string& type) const
{
    SdfPath primPath(path);
    TfToken primType = UsdGeomTokens->Scope;
    if (type == "Xform")
    {
        primType = UsdGeomTokens->Xform;
    }
    UsdPrim newPrim = m_usdMembers->stage()->DefinePrim(primPath, primType);
}

void UsdProcessor::addMesh(const std::string& path, SdfPath primPath)
{
    if (std::filesystem::exists(path))
    {
        auto assetStage = pxr::UsdStage::Open(path);
        std::string name;
        if (!primPath.IsEmpty())
        {
            name = primPath.GetName();
        }
        else
        {
            UsdPrim prim = assetStage->GetDefaultPrim();
            SdfPath primPath = prim.GetPath();
            name = primPath.GetName();
            if (name == "root")
            {
                // take child of root, don't want the top-most root
                prim = prim.GetChildren().front();
                primPath = prim.GetPath();
                name = primPath.GetName();
            }
        }

        UsdPrim meshes = m_usdMembers->stage()->GetPrimAtPath(MeshesPath);
        UsdPrim meshPrim = m_usdMembers->stage()->DefinePrim(MeshesPath.AppendPath(SdfPath(name)), UsdGeomTokens->Xform);
        meshPrim.GetReferences().AddReference(path, primPath);
		meshPrim.SetCustomDataByKey(QUBED::Keys::Type, QUBED::Values::MeshType);
    }
}

void UsdProcessor::bakeStage(Node& root, Services& services)
{
    //UsdPrim rootPrim = m_usdMembers->stage()->GetPrimAtPath(WorldPath);
    UsdPrim rootPrim = m_usdMembers->stage()->GetDefaultPrim();
    for (auto child : rootPrim.GetChildren())
    {
        processPrim(this, child, root, services, m_meshes);
    }
    d3d12rs::D3D12RenderSystem *renderer = services.compSys().getSystemRegistry().getSystem<
        d3d12rs::D3D12RenderSystem>();
    renderer->executeAssetCopyOps();
}

void UsdProcessor::addBrush(pxr::SdfPath meshPath)
{
    UsdPrim meshPrim = stage()->GetPrimAtPath(meshPath);
    const std::string meshName = meshPrim.GetName();
    const std::string brushName = std::format("{}", meshName);
    SdfPath brushPath = BrushesPath.AppendPath(SdfPath(brushName));

    UsdPrim brushPrim = stage()->DefinePrim(brushPath, UsdGeomTokens->Xform);
    brushPrim.GetReferences().AddReference(SdfReference("", meshPath));
    brushPrim.SetInstanceable(true);
    brushPrim.SetCustomDataByKey(QUBED::Keys::Type, QUBED::Values::BrushType);
}

void UsdProcessor::placeBrush(pxr::SdfPath brushPath)
{
    UsdPrim brushPrim = stage()->GetPrimAtPath(brushPath);
    const std::string brushName = brushPrim.GetName();
    const std::string objName = std::format("{}_01", brushName);
    SdfPath objPath = WorldPath.AppendPath(SdfPath(objName));

    stage()->SetEditTarget(m_usdMembers->worldLayer());

    UsdPrim prim = stage()->DefinePrim(objPath, UsdGeomTokens->Xform);
    prim.GetReferences().AddInternalReference(brushPath);
    prim.SetInstanceable(true);

    stage()->SetEditTarget(stage()->GetRootLayer());

    // auto primSpecHandle = SdfCreatePrimInLayer(m_usdMembers->worldLayer(), objPath);
    // primSpecHandle->SetTypeName(UsdGeomTokens->Xform);
    // primSpecHandle->GetReferenceList().Add(brushPath.GetAsString());
    // m_usdMembers->worldLayer()->Save();
}

void work(UsdPrim prim, const Prim &parent, std::vector<Prim> &flatList, uint32_t &currentId)
{
    flatList.push_back(parent);
    
    for (UsdPrim child : prim.GetChildren())
    {
		const Prim childPrim {
			.id = currentId++,
			.parentId = parent.id,
			.name = child.GetName().GetString().c_str()
		};
        work(child, childPrim, flatList, currentId);
    }
}

void UsdProcessor::flatten(std::vector<Prim> &flatList, bool useDefaultPrim)
{
    uint32_t currentId = 1;
    
    UsdPrim root = useDefaultPrim ? stage()->GetDefaultPrim() : stage()->GetPseudoRoot();
    const Prim rootPrim {
        .id = currentId++,
        .parentId = 0,
        .name = root.GetName().GetString().c_str()
    };
    work(root, rootPrim, flatList, currentId);
}
