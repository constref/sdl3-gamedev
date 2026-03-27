#pragma once
#include <QObject>
#include <tooling/usd/usdstagelistener.h>

class TreeViewStageListener : public QObject, public usd::UsdStageListener
{
    Q_OBJECT

public:
    void notifyPrimChanged(pxr::SdfPath path, pxr::UsdStageRefPtr stage) override;

signals:
    void primChanged(pxr::SdfPath path, pxr::UsdStageRefPtr stage);
};
