#pragma once

#include <fstream>
#include <assets/assetid.h>
#include <DirectXMath.h>
#include <memory>
#include <rendering/mesh.h>

#include <uuid.h>

namespace persistence
{
    struct Project
    {
    };

    struct Node
    {
        uint32_t id = 0;
        uint32_t parentId = 0;
        AssetId meshId;
        DirectX::XMFLOAT3 position = {};
        DirectX::XMFLOAT3 rotation = {};
    };

    struct MeshHeader
    {
	uuids::uuid id;
        uint32_t subMeshCount = 0;
    };

    struct SubMeshHeader
    {
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
    };

    std::ofstream createFile(const std::string &filepath);
    void finish(std::ofstream &file);

    std::unique_ptr<Mesh> readMeeshFile(const std::string &assetId);
    void writeMeshFile(const std::string &assetId, const Mesh &mesh);
}
