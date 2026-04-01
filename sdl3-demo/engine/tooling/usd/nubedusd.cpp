#include "nubedusd.h"
#include <tooling/usd/usdprocessor.h>

usd::UsdProcessor* CreateStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->createStage(path);
    usdProc->flatten();
    return usdProc;
}

usd::UsdProcessor* OpenStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->openStage(path);
    return usdProc;
}

void SaveStage(usd::UsdProcessor *proc)
{
    proc->saveStage();
}

uint32_t BuildFlatTree(usd::UsdProcessor *proc)
{
    //proc->flatten();
    return 0;
}
