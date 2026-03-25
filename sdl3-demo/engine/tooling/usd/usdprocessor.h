#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <systems/system.h>
#include <components/meshcomponent.h>
#include <rendering/mesh.h>

#include <tooling/usd.h>

class Node;

namespace usd
{
class StageProcessor;

struct PrimGeo
{
    std::unique_ptr<Mesh> mesh;
    GPUMeshHandle gpuHandle;
};

class UsdProcessor
{
    std::unordered_map<std::string, PrimGeo> m_meshes;
    StageProcessor *m_stageProc;
    pxr::UsdStageRefPtr m_stage;

public:
    UsdProcessor();
    ~UsdProcessor();

    pxr::UsdStageRefPtr stage() const { return m_stage; }
    void createStage(const std::string &path);
    void saveStage() const;
    void addLayer(const std::string &layerPath);
    void openStage(const std::string &usdPath);
    void addPrim(const std::string &path, const std::string &type) const;
    void bakeStage(Node &root, Services &services);
};
}
