#pragma once

#include <vector>

class Renderer
{
public:
    virtual void updateGPUTextures() = 0;
    virtual std::vector<uint64_t> getSharedTextureHandles(int editorPID) const = 0;
};
