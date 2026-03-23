#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <systems/system.h>
#include <components/meshcomponent.h>
#include <rendering/mesh.h>

class Node;
class StageProcessor;

struct PrimGeo
{
    std::unique_ptr<Mesh> mesh;
    GPUMeshHandle gpuHandle;
};

namespace usd
{
class UsdProcessor
{
    std::unordered_map<std::string, PrimGeo> m_meshes;
    StageProcessor *m_noticeHandler;

public:
    UsdProcessor();
    ~UsdProcessor();
    void createStage(const std::string &path);
    void saveStage();
    void addLayer(const std::string &layerPath);
    void openStage(const std::string &usdPath);
    void addPrim(const std::string &path, const std::string &type);
    void bakeStage(Node &root, Services &services);
};
}
