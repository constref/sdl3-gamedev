#pragma once

#include <vector>
#include "vertex.h"

struct SubMesh
{
    using IndexType = uint16_t;
    std::vector<Vertex> vertices;
    std::vector<IndexType> indices;

    size_t vertexElementSize() const
    {
	return sizeof(decltype(vertices)::value_type);
    }
    size_t indexElementSize() const
    {
	return sizeof(decltype(indices)::value_type);
    }

    size_t vertexByteSize() const
    {
	return vertexElementSize() * vertices.size();
    }

    size_t indexByteSize() const
    {
	return indexElementSize() * indices.size();
    }      
};

class Mesh
{
    size_t m_vertexCount = 0;
    size_t m_indexCount = 0;
    std::vector<SubMesh> m_subMeshes;
public:
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
    size_t indicesByteSize() const { return m_indexCount * sizeof(SubMesh::IndexType); }
};
