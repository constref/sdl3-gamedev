#include "treeviewstagelistener.h"

void TreeViewStageListener::notifyPrimChanged(pxr::SdfPath path, pxr::UsdStageRefPtr stage)
{
    emit primChanged(path, stage);
}
