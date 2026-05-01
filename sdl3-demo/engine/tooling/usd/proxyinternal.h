#pragma once

#include <tooling/usd.h>
#include <tooling/usd/common.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/notice.h>
#include <usd.pb.h>

using namespace pxr;

namespace usd
{
    class ProxyInternal : public pxr::TfWeakBase
    {
        pxr::UsdStageRefPtr m_stage;
        ObjectsChangedFunc m_objectsChangedCallback;
        uint8_t m_msgBuffer[1024];
        std::vector<std::string> m_resyncedPaths;

    public:
        ProxyInternal(const UsdStageRefPtr& stage, ObjectsChangedFunc objectsChangedCallback)
        {
            m_stage = stage;
            if (objectsChangedCallback)
            {
                m_objectsChangedCallback = objectsChangedCallback;
                TfNotice::Register(TfCreateWeakPtr(this), &ProxyInternal::onObjectsChanged);
            }
        }

        pxr::UsdStageRefPtr stage() const
        {
            return m_stage;
		}

        void onObjectsChanged(const UsdNotice::ObjectsChanged& notice)
        {
            for (auto& path : notice.GetResyncedPaths())
            {
                m_resyncedPaths.push_back(path.GetString());
            }
        }

        void flushChanged()
        {
            NUBE::USD::ObjectsChanged msg;
            for (std::string& path : m_resyncedPaths)
            {
                msg.add_resynced_paths(path);
            }

            size_t byteSize = msg.ByteSizeLong();
            if (msg.SerializeToArray(m_msgBuffer, byteSize))
            {
                m_objectsChangedCallback(m_msgBuffer, byteSize);
            }
        }
    };
}
