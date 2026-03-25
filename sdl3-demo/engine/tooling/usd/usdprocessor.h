#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <systems/system.h>
#include <components/meshcomponent.h>
#include <pxr/pxr.h>
#include <pxr/usd/usd/common.h>
#include <rendering/mesh.h>

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

public:
    UsdProcessor();
    ~UsdProcessor();

    pxr::UsdStageRefPtr stage() const;
    void createStage(const std::string &path) const;
    void saveStage() const;
    void addLayer(const std::string &layerPath);
    void openStage(const std::string &usdPath) const;
    void addPrim(const std::string &path, const std::string &type) const;
    void bakeStage(Node &root, Services &services);
};
}
