#include "nubedusd.h"
#include <tooling/usd/usdprocessor.h>

usd::UsdProcessor *CreateStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->createStage(path);
    return usdProc;
}

usd::UsdProcessor *OpenStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->openStage(path);
    return usdProc;
}

void SaveStage(usd::UsdProcessor *proc)
{
    proc->saveStage();
}

void DestroyUsdProcessor(usd::UsdProcessor *proc)
{
    delete proc;
}

PrimList *BuildPrimList(usd::UsdProcessor *proc)
{
    PrimList *list = new PrimList();
    proc->flatten(*list);
    return list;
}

usd::Prim *GetPrimListData(PrimList *list, uint32_t *outSize)
{
    *outSize = static_cast<uint32_t>(list->size());
    return list->data();
}

void DestroyPrimList(PrimList *list)
{
    delete list;
}
