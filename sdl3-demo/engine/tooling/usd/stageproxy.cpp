#include "stageproxy.h"

#include <usd.pb.h>
#include <pxr/usd/usdGeom/tokens.h>

using namespace usd;
using namespace pxr;

StageProxy::StageProxy(const UsdStageRefPtr &stage, ObjectsChangedFunc objectsChangedCallback)
{
    m_stage = stage;
    m_objectsChangedCallback = objectsChangedCallback;
    TfNotice::Register(TfCreateWeakPtr(this), &StageProxy::onObjectsChanged);
}

UsdStageRefPtr StageProxy::stage()
{
    return m_stage;
}

void StageProxy::onObjectsChanged(const UsdNotice::ObjectsChanged &notice)
{
    for (auto &path: notice.GetResyncedPaths())
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

void StageProxy::flushChanged()
{
    NUBE::USD::ObjectsChanged msg;
    for (std::string &path : m_resyncedPaths)
    {
        msg.add_resynced_paths(path);
    }
    
    size_t byteSize = msg.ByteSizeLong();
    if (msg.SerializeToArray(m_msgBuffer, byteSize))
    {
        m_objectsChangedCallback(m_msgBuffer, byteSize);
    }
}

void StageProxy::work(UsdPrim prim, const Prim &parent, std::vector<Prim> &flatList, uint32_t &currentId)
{
    flatList.push_back(parent);
    
    for (UsdPrim child : prim.GetChildren())
    {
        const Prim childPrim {
            .id = currentId++,
            .parentId = parent.id,
            .name = child.GetName().GetString().c_str(),
            .type = child.GetTypeName().GetText(),
            .path = child.GetPath().GetString().c_str()
        };
        work(child, childPrim, flatList, currentId);
    }
}

void StageProxy::flatten(std::vector<Prim> &flatList, bool useDefaultPrim)
{
    uint32_t currentId = 1;
    
    UsdPrim root = useDefaultPrim ? stage()->GetDefaultPrim() : stage()->GetPseudoRoot();
    const Prim rootPrim {
        .id = currentId++,
        .parentId = 0,
        .name = root.GetName().GetString().c_str(),
        .type = root.GetTypeName().GetText(),
        .path = root.GetPath().GetString().c_str()
    };
    work(root, rootPrim, flatList, currentId);
}

void StageProxy::addMesh(const char *assetPath, const char *primPath)
{
    const std::string name = SdfPath(primPath).GetName();
    {
        SdfChangeBlock changeBlock;
        auto layer = stage()->GetEditTarget().GetLayer();
        SdfPath meshPath = SdfPath("/Library/Meshes").AppendChild(TfToken(name));
                
        auto spec = SdfCreatePrimInLayer(layer, meshPath);
        spec->SetSpecifier(SdfSpecifierDef);
        spec->SetTypeName(UsdGeomTokens->Xform);
        SdfReference ref(assetPath, SdfPath(primPath));
        spec->GetReferenceList().Add(ref);
    }
}

void StageProxy::addBrush(const char *meshPath)
{
    const std::string name = SdfPath(meshPath).GetName();
    {
        SdfChangeBlock changeBlock;
        auto layer = stage()->GetEditTarget().GetLayer();
        SdfPath brushPath = SdfPath("/Library/Brushes").AppendChild(TfToken(name));
                
        auto spec = SdfCreatePrimInLayer(layer, brushPath);
        spec->SetSpecifier(SdfSpecifierDef);
        spec->SetTypeName(UsdGeomTokens->Xform);
        SdfReference ref("", SdfPath(meshPath));
        spec->GetReferenceList().Add(ref);
    }
}
