#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <components/meshcomponent.h>

class Mesh;
class Node;
class Services;

namespace usd
{
class UsdMembers;
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

struct PrimGeo
{
    std::unique_ptr<Mesh> mesh;
    GPUMeshHandle gpuHandle;
};

class UsdProcessor
{
    std::unordered_map<std::string, PrimGeo> m_meshes;
    std::unique_ptr<UsdMembers> m_usdMembers;

public:
    UsdProcessor(std::shared_ptr<UsdStageListener> listener = nullptr);
    ~UsdProcessor();

    void openStage(const std::string &usdPath);
    void bakeStage(Node &root, Services &services);
};
}
