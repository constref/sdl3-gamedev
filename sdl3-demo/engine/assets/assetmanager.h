#pragma once

#include <string>
#include <memory>
#include <unordered_map>

class Mesh;

class AssetManager
{
    std::unordered_map<std::string, std::unique_ptr<Mesh>> assetMap;
public:
    
};
