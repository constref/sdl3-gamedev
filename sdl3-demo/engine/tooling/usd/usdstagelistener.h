#pragma once

#include <tooling/usd.h>
#include <pxr/usd/sdf/path.h>

namespace usd
{

class UsdStageListener
{
public:
    virtual void notifyPrimChanged(pxr::SdfPath primPath, pxr::UsdStageRefPtr stage) = 0;
};

}
