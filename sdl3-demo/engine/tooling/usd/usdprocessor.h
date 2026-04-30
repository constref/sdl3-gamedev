#pragma once
#include <memory>
#include <string>

class Mesh;
class Node;
class Services;

namespace usd
{
class StageProxy;
class UsdStageListener;
    
#pragma pack(push, 8)
struct Prim
{
    uint32_t id;
    uint32_t parentId;
    const char *name;
    const char *type;
    const char *path;
};
#pragma pack(pop)

class UsdProcessor
{

public:
    UsdProcessor();
    // std::string findMesh(std::string assetPath, std::string startPrim);
    void bakeStage(StageProxy &stage, const std::string &nubPath);
};
}
