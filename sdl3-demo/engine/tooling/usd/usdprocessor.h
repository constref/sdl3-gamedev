#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <components/meshcomponent.h>

#include <tooling/usd.h>

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

    pxr::UsdStageRefPtr stage();
    void createStage(const std::string &path);
    void saveStage() const;
    void addLayer(const std::string &layerPath);
    void openStage(const std::string &usdPath);
    void addPrim(const std::string &path, const std::string &type) const;
    void addMesh(const std::string &path, pxr::SdfPath primPath);
    void bakeStage(Node &root, Services &services);
    void addBrush(pxr::SdfPath meshPath);
    void placeBrush(pxr::SdfPath brushPath);
    void flatten(std::vector<Prim> &flatList, bool usdDefaultPrim);
};
}
