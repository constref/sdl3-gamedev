#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include  <uuid.h>

class Mesh;

class AssetManager
{
    std::unordered_map<uuids::uuid, std::unique_ptr<Mesh>> assetMap;

public:
    void loadMesh(uuids::uuid assetId, std::unique_ptr<Mesh> mesh);
};
