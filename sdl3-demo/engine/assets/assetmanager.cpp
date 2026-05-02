#include "assetmanager.h"
#include <rendering/mesh.h>

void AssetManager::loadMesh(uuids::uuid assetId, std::unique_ptr<Mesh> mesh)
{
    assetMap.insert({assetId, std::move(mesh)});
}
