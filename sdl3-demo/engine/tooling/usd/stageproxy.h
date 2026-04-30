#pragma once

#include "../usd.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/prim.h>
#include "common.h"
#include "usdprocessor.h"


namespace usd
{
    class StageProxy : public pxr::TfWeakBase
    {
        pxr::UsdStageRefPtr m_stage;
        ObjectsChangedFunc m_objectsChangedCallback;
        uint8_t m_msgBuffer[1024];
        std::vector<std::string> m_resyncedPaths;
        
    public:
        StageProxy(const pxr::UsdStageRefPtr &stage, ObjectsChangedFunc objectsChangedCallback);
        ObjectsChangedFunc objectsChangedCallback() const;
        
        pxr::UsdStageRefPtr stage();
        void onObjectsChanged(const pxr::UsdNotice::ObjectsChanged &notice);
        void flushChanged();
        
        void work(pxr::UsdPrim prim, const Prim &parent, std::vector<Prim> &flatList, uint32_t &currentId);
        void flatten(std::vector<Prim> &flatList, bool useDefaultPrim);
        void addMesh(const std::string &assetId, const std::string &assetPath, const std::string &primPath);
        void createBrush(const std::string &meshPath);
        void placeBrush(const std::string &brushPath);
    };
}
