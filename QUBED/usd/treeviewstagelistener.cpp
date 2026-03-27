//
// Created by nikol on 3/27/2026.
//

#include "treeviewstagelistener.h"

void TreeViewStageListener::notifyPrimChanged(pxr::SdfPath path, pxr::UsdStageRefPtr stage)
{
    emit primChanged(path, stage);
}
