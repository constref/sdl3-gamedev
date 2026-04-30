#pragma once

#include <vector>

#include <assetid.h>
#include "vertex.h"

struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	size_t vertexElementSize() const
	{
		return sizeof(decltype(vertices)::value_type);
	}
	size_t indexElementSize() const
	{
		return sizeof(decltype(indices)::value_type);
	}
};

class Mesh
{
	AssetId m_id;
	size_t m_vertexCount = 0;
	size_t m_indexCount = 0;
	std::vector<SubMesh> m_subMeshes;
public:
	Mesh(AssetId id) : m_id(id) {}
	bool operator==(const Mesh &other) const
	{
		return m_id == other.m_id;
	}
	
	AssetId id() const { return m_id; }
	void addSubmesh(SubMesh &&subMesh)
	{
		m_vertexCount += subMesh.vertices.size();
		m_indexCount += subMesh.indices.size();
		m_subMeshes.push_back(subMesh);
	}
	const auto &subMeshes() const
	{
		return m_subMeshes;
	}
	size_t vertexCount() const { return m_vertexCount; }
	size_t indexCount() const { return m_indexCount; }
	size_t verticesByteSize() const { return m_vertexCount * sizeof(Vertex); }
	size_t indicesByteSize() const { return m_indexCount * sizeof(uint16_t); }
};
