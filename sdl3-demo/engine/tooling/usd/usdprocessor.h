#pragma once
#include <string>

class Mesh;

namespace usd
{
class StageProxy;
class UsdStageListener;

class UsdProcessor
{
public:
    UsdProcessor();
    // std::string findMesh(std::string assetPath, std::string startPrim);
    void bakeStage(StageProxy &stage, const std::string &nubPath);

};
}
