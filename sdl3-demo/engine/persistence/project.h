#pragma once

#include <fstream>
#include <assets/assetid.h>
#include <DirectXMath.h>

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
    
    struct Mesh
    {
        AssetId id;
        uint32_t subMeshCount = 0;
    };
    
    struct SubMesh
    {
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
    };
    
    std::ofstream createFile(const std::string &filepath);
    void finish(std::ofstream &file);
}

