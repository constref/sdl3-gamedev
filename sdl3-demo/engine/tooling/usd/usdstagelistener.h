#pragma once

#include <tooling/usd.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/notice.h>

namespace usd
{
class UsdStageListener : public pxr::TfWeakBase
{
public:
	virtual ~UsdStageListener() = default;

	virtual void notifyPrimChanged(pxr::SdfPath primPath, pxr::UsdStageRefPtr stage) = 0;

	void onObjectsChanged(const pxr::UsdNotice::ObjectsChanged &notice)
	{
		for (auto &path: notice.GetResyncedPaths())
		{
			notifyPrimChanged(path, notice.GetStage());
		}
	}
};
}
