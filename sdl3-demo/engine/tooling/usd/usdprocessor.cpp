#include "usdprocessor.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/tokens.h>

void USDProcessor::loadStage()
{
	// This doesn't need a file on disk to work
    auto stage = pxr::UsdStage::CreateInMemory();
	pxr::VtValue upAxis;
	stage->GetMetadata(pxr::UsdGeomTokens->upAxis, &upAxis);
}
