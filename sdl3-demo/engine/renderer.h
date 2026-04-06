#pragma once

#include <vector>

class Renderer
{
    virtual std::vector<uint64_t> getSharedTextureHandles(int editorPID) const = 0;
};
