#include "stageproxy.h"

#include <usd.pb.h>
#include "../usd.h"
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/prim.h>

using namespace usd;
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

            // NUBE::USD::ObjectsChanged msg;
            // for (auto &path: notice.GetResyncedPaths())
            // {
            //     msg.add_resynced_paths(path.GetString());
            // }
            //
            // size_t byteSize = msg.ByteSizeLong();
            // if (msg.SerializeToArray(m_msgBuffer, byteSize))
            // {
            //     m_objectsChangedCallback(m_msgBuffer, byteSize);
            // }
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

StageProxy::StageProxy(std::unique_ptr<ProxyInternal> proxyInternal)
{
    m_internal = std::move(proxyInternal);
}

StageProxy::~StageProxy()
{
}

StageProxy* StageProxy::create(const std::string& path, ObjectsChangedFunc objectsChangedCallback)
{
    UsdStageRefPtr stage = UsdStage::CreateNew(path);
    stage->DefinePrim(SdfPath("/Library"), UsdGeomTokens->Scope);
    stage->DefinePrim(SdfPath("/Library/Meshes"), UsdGeomTokens->Scope);
    stage->DefinePrim(SdfPath("/Library/Brushes"), UsdGeomTokens->Scope);
    UsdPrim world = stage->DefinePrim(SdfPath("/World"), UsdGeomTokens->Xform);
    stage->SetDefaultPrim(world);
	return new StageProxy(std::make_unique<ProxyInternal>(stage, objectsChangedCallback));
}

StageProxy* StageProxy::open(const std::string& path, ObjectsChangedFunc objectsChangedCallback)
{
    UsdStageRefPtr stage = UsdStage::Open(path);
	return new StageProxy(std::make_unique<ProxyInternal>(stage, objectsChangedCallback));
}

void StageProxy::flushChanged() const
{
	m_internal->flushChanged();
}

void StageProxy::save() const
{
	m_internal->stage()->Save();
}

void work(UsdPrim prim, const Prim& parent, std::vector<Prim>& flatList, uint32_t& currentId)
{
    flatList.push_back(parent);

    for (UsdPrim child : prim.GetChildren())
    {
        const Prim childPrim{
            .id = currentId++,
            .parentId = parent.id,
            .name = child.GetName().GetString().c_str(),
            .type = child.GetTypeName().GetText(),
            .path = child.GetPath().GetString().c_str()
            };
        work(child, childPrim, flatList, currentId);
    }
}

void StageProxy::flatten(std::vector<Prim>& flatList, bool useDefaultPrim) const
{
    uint32_t currentId = 1;

    UsdPrim root = useDefaultPrim ? m_internal->stage()->GetDefaultPrim() : m_internal->stage()->GetPseudoRoot();
    const Prim rootPrim{
        .id = currentId++,
        .parentId = 0,
        .name = root.GetName().GetString().c_str(),
        .type = root.GetTypeName().GetText(),
        .path = root.GetPath().GetString().c_str()
        };
    work(root, rootPrim, flatList, currentId);
}

void StageProxy::addMesh(const std::string& assetId, const std::string& assetPath, const std::string& primPath) const
{
    const std::string name = SdfPath(primPath).GetName();
    {
        SdfChangeBlock changeBlock;
        auto layer = m_internal->stage()->GetEditTarget().GetLayer();
        SdfPath meshPath = SdfPath("/Library/Meshes").AppendChild(TfToken(name));

        auto spec = SdfCreatePrimInLayer(layer, meshPath);
        spec->SetSpecifier(SdfSpecifierDef);
        spec->SetTypeName(UsdGeomTokens->Xform);

        // associate the 
        SdfAttributeSpecHandle attrAssetId =
            SdfAttributeSpec::New(spec, "assetInfo:assetId", SdfValueTypeNames->String);
        attrAssetId->SetDefaultValue(VtValue(assetId));

        SdfReference ref(assetPath, SdfPath(primPath));
        spec->GetReferenceList().Add(ref);
    }
}

void StageProxy::createBrush(const std::string& meshPath) const
{
    const std::string name = SdfPath(meshPath).GetName();
    {
        SdfChangeBlock changeBlock;
        auto layer = m_internal->stage()->GetEditTarget().GetLayer();
        SdfPath brushPath = SdfPath("/Library/Brushes").AppendChild(TfToken(std::format("brush_{}", name)));

        auto spec = SdfCreatePrimInLayer(layer, brushPath);
        spec->SetSpecifier(SdfSpecifierDef);
        spec->SetTypeName(UsdGeomTokens->Xform);
        SdfReference ref("", SdfPath(meshPath));
        spec->GetReferenceList().Add(ref);
    }
}

void StageProxy::placeBrush(const std::string& brushPath) const
{
    const std::string name = SdfPath(brushPath).GetName();
    {
        SdfChangeBlock changeBlock;
        auto layer = m_internal->stage()->GetEditTarget().GetLayer();
        SdfPath primPath = SdfPath("/World").AppendChild(TfToken(std::format("{}_001", name)));

        auto spec = SdfCreatePrimInLayer(layer, primPath);
        spec->SetSpecifier(SdfSpecifierDef);
        spec->SetTypeName(UsdGeomTokens->Xform);
        spec->SetInstanceable(true);
        SdfReference ref("", SdfPath(brushPath));
        spec->GetReferenceList().Add(ref);
    }
}
