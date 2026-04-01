#pragma once
#include <string>

#include "usdprocessor.h"

namespace usd { class UsdProcessor; };

#define DLL_EXPORT
#ifdef DLL_EXPORT
	#define DLL_API __declspec(dllexport)
#else
	#define DLL_API __declspec(dllimport)
#endif

extern "C"
{

DLL_API usd::UsdProcessor *CreateStage(const char *path);
DLL_API usd::UsdProcessor *OpenStage(const char *path);
DLL_API void SaveStage(usd::UsdProcessor *proc);
DLL_API uint32_t BuildFlatTree(usd::UsdProcessor *proc);

}

