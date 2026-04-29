#pragma once

#include <fstream>
#include <assetid.h>

namespace persistence
{
    struct Project
    {
    };
    
    struct Node
    {
        uint32_t id;
        uint32_t parentId;
        AssetId meshId;
    };
    
    struct Mesh
    {
        AssetId id;
        uint32_t subMeshCount;
    };
    
    struct SubMesh
    {
        uint32_t vertexCount;
        uint32_t indexCount;
    };
    
    std::ofstream createFile(const std::string &filepath);
    void finish(std::ofstream &file);
}

