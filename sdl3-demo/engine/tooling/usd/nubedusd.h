#pragma once
#include <string>

#include "usdprocessor.h"

namespace usd
{
    class UsdProcessor;
};

#define DLL_EXPORT
#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" {
using PrimList = std::vector<usd::Prim>;

DLL_API usd::UsdProcessor *CreateStage(const char *path);
DLL_API usd::UsdProcessor *OpenStage(const char *path);
DLL_API void SaveStage(usd::UsdProcessor *proc);
DLL_API void DestroyUsdProcessor(usd::UsdProcessor *proc);

DLL_API PrimList *BuildPrimList(usd::UsdProcessor *proc);
DLL_API usd::Prim *GetPrimListData(PrimList *list, uint32_t *outSize);
DLL_API void DestroyPrimList(PrimList *list);
}
