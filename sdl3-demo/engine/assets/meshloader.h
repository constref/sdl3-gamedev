#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Mesh;

struct LoadResult
{
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
};

namespace assets
{
LoadResult loadGltf(const std::string &path);
}
