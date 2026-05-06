#include "stageproxy.h"

#include "../usd.h"
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <usd.pb.h>

#include <tooling/usd/proxyinternal.h>
#include <tooling/usd/usdprocessor.h>

using namespace usd;
using namespace pxr;

StageProxy::StageProxy(std::unique_ptr<ProxyInternal> proxyInternal) { m_internal = std::move(proxyInternal); }

StageProxy::~StageProxy() {}

StageProxy *StageProxy::create(const std::string &path, ObjectsChangedFunc objectsChangedCallback)
{
    UsdStageRefPtr stage = UsdStage::CreateNew(path);
    stage->DefinePrim(SdfPath("/Library"), UsdGeomTokens->Scope);
    stage->DefinePrim(SdfPath("/Library/Meshes"), UsdGeomTokens->Scope);
    stage->DefinePrim(SdfPath("/Library/Brushes"), UsdGeomTokens->Scope);
    UsdPrim world = stage->DefinePrim(SdfPath("/World"), UsdGeomTokens->Xform);
    stage->SetDefaultPrim(world);
    return new StageProxy(std::make_unique<ProxyInternal>(stage, objectsChangedCallback));
}

StageProxy *StageProxy::open(const std::string &path, ObjectsChangedFunc objectsChangedCallback)
{
    UsdStageRefPtr stage = UsdStage::Open(path);
    return new StageProxy(std::make_unique<ProxyInternal>(stage, objectsChangedCallback));
}

void StageProxy::flushChanged() const { m_internal->flushChanged(); }

void StageProxy::save() const { m_internal->stage()->Save(); }

void work(UsdPrim prim, const Prim &parent, std::vector<Prim> &flatList, uint32_t &currentId)
{
    flatList.push_back(parent);

    for (UsdPrim child : prim.GetChildren())
    {
        const Prim childPrim{.id = currentId++,
                             .parentId = parent.id,
                             .name = child.GetName().GetString().c_str(),
                             .type = child.GetTypeName().GetText(),
                             .path = child.GetPath().GetString().c_str()};
        work(child, childPrim, flatList, currentId);
    }
}

void StageProxy::flatten(std::vector<Prim> &flatList, bool useDefaultPrim) const
{
    uint32_t currentId = 1;

    UsdPrim root = useDefaultPrim ? m_internal->stage()->GetDefaultPrim() : m_internal->stage()->GetPseudoRoot();
    const Prim rootPrim{.id = currentId++,
                        .parentId = 0,
                        .name = root.GetName().GetString().c_str(),
                        .type = root.GetTypeName().GetText(),
                        .path = root.GetPath().GetString().c_str()};
    work(root, rootPrim, flatList, currentId);
}

void StageProxy::addMesh(const std::string &assetId, const std::string &assetPath, const std::string &primPath) const
{
    const std::string name = SdfPath(primPath).GetName();
    {
        SdfChangeBlock changeBlock;
        auto layer = m_internal->stage()->GetEditTarget().GetLayer();
        SdfPath meshPath = SdfPath("/Library/Meshes").AppendChild(TfToken(name));

        auto spec = SdfCreatePrimInLayer(layer, meshPath);
        spec->SetSpecifier(SdfSpecifierDef);
        spec->SetTypeName(UsdGeomTokens->Xform);

        // associate the mesh prim with the asset ID
        SdfAttributeSpecHandle attrAssetId =
            SdfAttributeSpec::New(spec, "assetInfo:assetId", SdfValueTypeNames->String);
        attrAssetId->SetDefaultValue(VtValue(assetId));

        SdfReference ref(assetPath, SdfPath(primPath));
        spec->GetReferenceList().Add(ref);
    }
}

std::unique_ptr<Mesh> StageProxy::bakeMesh(const std::string &assetId, const std::string &primPath) const
{
    // bake the mesh
    auto assetPrim = m_internal->stage()->GetPrimAtPath(SdfPath(primPath));
    for (UsdPrim prim : assetPrim.GetAllDescendants())
    {
        if (prim.IsA<UsdGeomMesh>())
        {
            UsdProcessor processor;
            return processor.processMesh(UsdGeomMesh(prim));
        }
    }
    return nullptr;
}

void StageProxy::createBrush(const std::string &meshPath) const
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

void StageProxy::placeBrush(const std::string &brushPath) const
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

std::string usd::StageProxy::getProperty(const std::string &primPath, const std::string &propName)
{
    UsdPrim prim = m_internal->stage()->GetPrimAtPath(SdfPath(primPath));
    auto attr = prim.GetAttribute(TfToken("assetInfo:assetId"));
    std::string assetId;
    attr.Get(&assetId);
    return assetId;
}
