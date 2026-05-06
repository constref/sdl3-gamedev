#include "assetmanager.h"
#include <rendering/mesh.h>

void AssetManager::loadMesh(uuids::uuid assetId, std::unique_ptr<Mesh> mesh)
{
    assetMap.insert({assetId, std::move(mesh)});
}

Mesh *AssetManager::getMesh(const uuids::uuid &assetId)
{
    auto itr = assetMap.find(assetId);
    if (itr == assetMap.end())
    {
        return nullptr;
    }
    return itr->second.get();
}
