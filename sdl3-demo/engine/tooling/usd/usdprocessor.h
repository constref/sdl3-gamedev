#pragma once
#include <memory>
#include <string>

#define NOMINMAX
#include <pxr/usd/usdGeom/mesh.h>

class Mesh;

namespace usd
{
class ProxyInternal;

class UsdProcessor
{
public:
    UsdProcessor();
    void bakeStage(ProxyInternal& proxy, const std::string& nubPath);

    std::unique_ptr<Mesh> processMesh(pxr::UsdGeomMesh mesh) const;

};
}
