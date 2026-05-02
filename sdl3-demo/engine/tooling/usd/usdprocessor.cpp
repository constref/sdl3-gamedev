#include "usdprocessor.h"

#include <filesystem>
#include <functional>

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
#include <pxr/usd/usdLux/sphereLight.h>

#include <logger.h>
#include <tooling/platformutils.h>
#include <tooling/usd/usdstagelistener.h>
#include <componentsystems.h>
#include <rendering/mesh.h>
#include <components/lightingcomponent.h>
#include "usdlogger.h"

#include <persistence/project.h>
#include <tooling/usd/proxyinternal.h>

using namespace pxr;
using namespace DirectX;
using namespace usd;

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


std::unique_ptr<Mesh> UsdProcessor::processMesh(UsdGeomMesh mesh) const
{
    Logger::info(this, std::format("Processing UsdGeomMesh:{}", mesh.GetPath().GetString()));
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
        Logger::error(this, "Error triangulating face normals");
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
    Logger::info(this, "Vertex data triangulated, generating vertex and index data");

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
    Logger::info(this, std::format("Submesh generated with {} vertices and {} indices", submeshVerts.size(),
                                   submeshIndices.size()));
    
    std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>();
    newMesh->addSubmesh(SubMesh(submeshVerts, submeshIndices));
    return std::move(newMesh);
}

static void processPrim(UsdProcessor *self, UsdPrim prim, uint32_t id, uint32_t parentId, std::vector<persistence::Node> &nodes,
    std::unordered_map<std::string, uint32_t> &nodeMap,  std::unordered_map<std::string, std::unique_ptr<Mesh>> &meshMap, uint32_t &meshIndex)
{
    const std::string pathString = prim.GetPath().GetString();
    Logger::info(self, std::format("Processing path: {}", pathString));

    // lambda to process geom mesh and attach component to runtime Node
    auto processGeomMesh = [self, &meshMap, &meshIndex](UsdGeomMesh meshPrim)
    {
        const std::string path = meshPrim.GetPath().GetString();
        Logger::info(self, std::format("Geomesh path: {}: ", path));
        
        auto meshItr = meshMap.find(path);
        if (meshItr == meshMap.end())
        {
            std::unique_ptr<Mesh> mesh = self->processMesh(meshPrim);
            auto [itr, added] = meshMap.insert({path, std::move(mesh)});
            meshItr = itr;
        }
    };

    auto extractTransform = [](UsdPrim prim, persistence::Node &node)
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
                    assert(got && "GOT!");
                node.position = DirectX::XMFLOAT3(translation[0], translation[1], -translation[2]);
            }
            else if (op.GetOpType() == UsdGeomXformOp::TypeRotateXYZ)
            {
                if (op.GetPrecision() == UsdGeomXformOp::PrecisionFloat)
                {
                    GfVec3f rotations;
                    bool got = op.Get<GfVec3f>(&rotations);
                    assert(got && "GOT!");
                    node.rotation = XMFLOAT3(-DirectX::XMConvertToRadians(rotations[0]),
                                             -DirectX::XMConvertToRadians(rotations[1]),
                                             DirectX::XMConvertToRadians(rotations[2]));
                }
            }
            else if (op.GetOpType() == UsdGeomXformOp::TypeScale)
            {
            }
        }
    };

    auto &typeInfo = prim.GetPrimTypeInfo();
    if (prim.GetTypeName() == UsdGeomTokens->Scope)
    {
        // scopes are only for organization
        if (prim.IsActive())
        {
            // TODO: Ensure no material should be inherited from this Scope prim
            for (UsdPrim child : prim.GetChildren())
            {
                processPrim(self, child, id, parentId, nodes, nodeMap, meshMap, meshIndex);
            }
        }
    }
    else
    {
        persistence::Node node;
        node.id = id++;
        node.parentId = parentId;
        extractTransform(prim, node);
        nodeMap.insert({pathString, node.id});
        
        if (typeInfo.GetTypeName() == UsdGeomTokens->Xform)
        {
            if (prim.IsInstance())
            {
                // refer to prim's prototype
                UsdPrim prototype = prim.GetPrototype();
                for (UsdPrim child : prototype.GetChildren())
                {
                    if (child.GetTypeName() == UsdGeomTokens->Mesh)
                    {
                        UsdGeomMesh meshPrim(child);
                        processGeomMesh(meshPrim);
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
                        processGeomMesh(meshPrim);
                    }
                    else
                    {
                        processPrim(self, child, id, node.id, nodes, nodeMap, meshMap, meshIndex);
                    }
                }
            }
        }
        else if (prim.GetTypeName() == UsdGeomTokens->Mesh)
        {
            UsdGeomMesh meshPrim(prim);
            processGeomMesh(meshPrim);
        }
        else if (prim.GetTypeName() == UsdLuxTokens->SphereLight)
        {
            // NodeHandle hNode = world.createNode();
            // Node &node = world.getNode(hNode);
            // parent.addChild(node);
            //
            // extractTransform(prim, node);
            // auto &lightComp = services.compSys().addComponent<LightingComponent>(node);
            // lightComp.setColor(DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
        }
        nodes.push_back(node);
    }
}

UsdProcessor::UsdProcessor()
{
}

void UsdProcessor::bakeStage(ProxyInternal &proxy, const std::string &nubPath)
{
    // generate baked data
    std::vector<persistence::Node> nodes;
    std::unordered_map<std::string, uint32_t> nodeMap;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshMap;
    
    uint32_t nodeId = 0;
    uint32_t meshIndex = 0;
    UsdPrim rootPrim = proxy.stage()->GetDefaultPrim();
    for (auto child : rootPrim.GetChildren())
    {
        processPrim(this, child, nodeId++, 0, nodes, nodeMap, meshMap, meshIndex);
    }
    
    // write the baked data to a file
    auto file = persistence::createFile(nubPath);
    
    // nodes
    uint32_t nodeCount = nodes.size();
    file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));
    for (auto n : nodes)
    {
        file.write(reinterpret_cast<const char*>(&n), sizeof(persistence::Node));
    }
    
    // meshes
    uint32_t meshCount = meshMap.size();
    file.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
    for (const auto &itr : meshMap)
    {
        // mesh header
        Mesh *mesh = itr.second.get();
        persistence::MeshHeader prMesh;
        prMesh.subMeshCount = mesh->subMeshes().size();
        file.write(reinterpret_cast<const char*>(&prMesh), sizeof(persistence::MeshHeader));
        
        // submeshes
        for (const auto &sm : mesh->subMeshes())
        {
            persistence::SubMeshHeader prSubMesh;
            prSubMesh.vertexCount = sm.vertices.size();
            prSubMesh.indexCount = sm.indices.size();
            file.write(reinterpret_cast<const char*>(&prSubMesh), sizeof(persistence::SubMeshHeader));
            
            // write out vertices
            for (const Vertex &v : sm.vertices)
            {
                file.write(reinterpret_cast<const char*>(&v), sizeof(Vertex));
            }
            
            // write out indices
            for (uint16_t idx : sm.indices)
            {
                file.write(reinterpret_cast<const char*>(&idx), sizeof(uint16_t));
            }
        }
    }
    
    persistence::finish(file);
}
