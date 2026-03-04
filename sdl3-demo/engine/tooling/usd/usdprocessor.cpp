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

#include <systems/d3d12/d3d12rendersystem.h>

class USDLogger : public pxr::TfDiagnosticMgr::Delegate
{
public:
	void IssueError(const pxr::TfError &err) override
	{
		Logger::error(this, std::format("{}", err.GetErrorCodeAsString()));
	}
	void IssueFatalError(const pxr::TfCallContext &context,
		const std::string &msg) override
	{
		Logger::error(this, std::format("USD Fatal Error: {}", msg));
	}
	void IssueStatus(const pxr::TfStatus &status) override
	{
		Logger::info(this, std::format("USD Status: {}", status.GetCommentary()));
	}
	void IssueWarning(const pxr::TfWarning &warning) override
	{
		Logger::warn(this, std::format("{}", warning.GetCommentary()));
	}
};

struct VertexId
{
	uint32_t index = UINT32_MAX;
	pxr::GfVec3f normal;
	pxr::GfVec2f uv;
	constexpr static float epsilon = 0.001f;

	bool operator==(const VertexId &o) const
	{
		const float nDot = pxr::GfDot(normal, o.normal);
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



static Mesh processMesh(USDProcessor *self, pxr::UsdGeomMesh mesh)
{
	using namespace pxr;
	using namespace DirectX;

	UsdGeomPrimvarsAPI primvarApi(mesh);
	VtArray<GfVec3f> points;
	VtArray<int> faceCounts;
	VtArray<int> indices;
	mesh.GetPointsAttr().Get(&points);
	mesh.GetFaceVertexCountsAttr().Get(&faceCounts);
	mesh.GetFaceVertexIndicesAttr().Get(&indices);

	HdMeshTopology topology(UsdGeomTokens->none, UsdGeomTokens->leftHanded, faceCounts, indices);
	HdMeshUtil meshUtil(&topology, mesh.GetPath());

	VtVec3iArray newIndices;
	VtIntArray primitiveParams;
	meshUtil.ComputeTriangleIndices(&newIndices, &primitiveParams);

	Mesh newMesh;
	std::vector<Vertex> submeshVerts;
	submeshVerts.reserve(points.size());
	std::vector<uint16_t> submeshIndices;
	submeshIndices.reserve(newIndices.size());

	// vertex normals
	VtArray<GfVec3f> usdNormals;
	mesh.GetNormalsAttr().Get(&usdNormals);

	// triangulate the normals from usd source
	VtValue triNormalsVal;
	HdMeshComputationResult triNormResult = meshUtil.ComputeTriangulatedFaceVaryingPrimvar(usdNormals.data(), usdNormals.size(), HdTypeFloatVec3, &triNormalsVal);
	if (triNormResult == HdMeshComputationResult::Error)
	{
		Logger::error(self, "Error triangulating face normals");
	}
	VtVec3fArray triNormals = triNormalsVal.Get<VtArray<GfVec3f>>();

	// get UVs (st)
	UsdGeomPrimvar stPrimvar = primvarApi.GetPrimvar(TfToken("st"));
	VtArray<GfVec2f> stList;
	stPrimvar.GetAttr().Get(&stList);
	VtValue stTriVal;
	HdMeshComputationResult triStResult = meshUtil.ComputeTriangulatedFaceVaryingPrimvar(stList.begin(), stList.size(), HdTypeFloatVec2, &stTriVal);
	VtVec2fArray triSts = stTriVal.Get<VtArray<GfVec2f>>();

	const uint32_t indexCount = newIndices.size() * 3;
	assert((triNormals.size() == indexCount && triSts.size() == indexCount) && "Normals / UVs and index counts must match");

	std::unordered_map<VertexId, uint32_t, VertexId::Hasher> vertexMap;

	// copy the vertices and indices into the submesh
	//for (const GfVec3f &p : points)
	//{
	//	Vertex v;
	//	v.position = DirectX::XMFLOAT3(p[0], p[1], p[2]);
	//	v.color = DirectX::XMFLOAT4(1, 1, 1, 1);
	//	v.uv = DirectX::XMFLOAT2(0, 0);
	//	submeshVerts.push_back(v);
	//}

	uint32_t indexPos = 0;
	submeshVerts.reserve(points.size());
	for (uint32_t i = 0; i < newIndices.size(); ++i)
	{
		const GfVec3i vec3idx = newIndices[i];
		for (short vi = 0; vi < 3; ++vi)
		{
			const uint32_t idx = vec3idx[vi];
			GfVec3f usdNorm = triNormals[indexPos];
			GfVec2f usdSt = triSts[indexPos];

			// generate a vertex ID to check against the hash
			usdNorm.Normalize();
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
				v.position = XMFLOAT3(points[idx][0], points[idx][1], points[idx][2]);
				v.normal = XMFLOAT3(ve.normal[0], ve.normal[1], ve.normal[2]);
				v.uv = XMFLOAT2(ve.uv[0], ve.uv[1]);
				v.color = XMFLOAT4(1, 1, 1, 1);
				submeshVerts.push_back(v);
			}
			submeshIndices.push_back(itr->second);
			indexPos++;
		}
	}
	newMesh.addSubmesh(SubMesh(submeshVerts, submeshIndices));
	return newMesh;
}

Mesh USDProcessor::loadStage()
{
	using namespace pxr;

	// Activate debug symbols programmatically (alternative to env var)
	USDLogger usdLogger;
	TfDiagnosticMgr::GetInstance().AddDelegate(&usdLogger);
	PlugRegistry &plugReg = pxr::PlugRegistry::GetInstance();

	const std::string usdPath = "S:\\projects\\constref\\sdl3-demo\\data\\usd\\ufo.usd";
	if (!std::filesystem::exists(usdPath))
	{
		throw std::runtime_error("Unable to find USD file");
	}
	auto stage = pxr::UsdStage::Open(usdPath);
	auto range = stage->Traverse();
	for (auto itr = range.begin(); itr != range.end(); ++itr)
	{
		UsdPrim prim = *itr;
		if (prim.IsInstance())
		{
			UsdPrim prototype = prim.GetPrototype();
			for (UsdPrim child : prototype.GetChildren())
			{
				if (child.GetTypeName() == UsdGeomTokens->Mesh)
				{
					UsdGeomMesh meshPrim(child);
					auto newMesh = processMesh(this, meshPrim);
					return newMesh;
				}
			}
		}
		auto pathStr = prim.GetPath().GetString();
		Logger::info(this, std::format("Traversing {}", pathStr));
		if (prim.GetTypeName() == UsdGeomTokens->Mesh)
		{
			Logger::info(this, "Loading mesh");

			UsdGeomMesh geomMesh(prim);
			VtArray<GfVec3f> points;
			geomMesh.GetPointsAttr().Get(&points);
			//UsdAttribute points = prim.GetAttribute(UsdGeomTokens->Points);
			Logger::info(this, "Triangulating USD prim points");
		}
	}
	return Mesh();
}
