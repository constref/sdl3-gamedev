#include "nubedusd.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <tooling/usd/usdprocessor.h>

using namespace pxr;
using namespace usd;

namespace usd
{
    class StageProxy
    {
        UsdStageRefPtr m_stage;
        
    public:
        StageProxy(const UsdStageRefPtr &stage)
        {
            m_stage = stage;
        }
        
        UsdStageRefPtr stage()
        {
            return m_stage;
        }
        
        void work(UsdPrim prim, const Prim &parent, std::vector<Prim> &flatList, uint32_t &currentId)
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

        void flatten(std::vector<Prim> &flatList, bool useDefaultPrim)
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
        
        void addMesh(const char *assetPath, const char *primPath)
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
        
        void addBrush(const char *meshPath)
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
    };
}

StageProxy *CreateProject(const char *path)
{
    UsdStageRefPtr stage = UsdStage::CreateNew(path);
    stage->DefinePrim(SdfPath("/Library"), UsdGeomTokens->Scope);
    stage->DefinePrim(SdfPath("/Library/Meshes"), UsdGeomTokens->Scope);
    stage->DefinePrim(SdfPath("/Library/Brushes"), UsdGeomTokens->Scope);
    UsdPrim world = stage->DefinePrim(SdfPath("/World"), UsdGeomTokens->Xform);
    stage->SetDefaultPrim(world);
    return new StageProxy(stage);
}

StageProxy *OpenProject(const char *path)
{
    UsdStageRefPtr stage = UsdStage::Open(path);
    return new StageProxy(stage);
}

void SaveProject(usd::StageProxy *proxy)
{
    proxy->stage()->Save();
}

void DestroyProxy(usd::StageProxy *proxy)
{
    delete proxy;
}

usd::StageProxy *OpenStage(const char *path)
{
    UsdStageRefPtr stage = UsdStage::Open(path);
    return new StageProxy(stage);
}

usd::UsdProcessor *CreateStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->createStage(path);
    return usdProc;
}

void DestroyUsdProcessor(usd::UsdProcessor *proc)
{
    delete proc;
}

PrimList *BuildPrimList(usd::StageProxy *proxy, bool useDefaultPrim)
{
    auto flatList = new std::vector<Prim>();
    proxy->flatten(*flatList, useDefaultPrim);
    return flatList;
}

usd::Prim *GetPrimListData(PrimList *list, uint32_t *outSize)
{
    *outSize = static_cast<uint32_t>(list->size());
    return list->data();
}

void DestroyPrimList(PrimList *list)
{
    delete list;
}

zmq::context_t *CreateContext()
{
    return new zmq::context_t();
}

void DestroyContext(zmq::context_t *context)
{
    delete context;
}

zmq::socket_t *CreateSocket(zmq::context_t *context, int socketType)
{
    zmq::socket_t *socket = new zmq::socket_t(*context, socketType);
    return socket;
}

void BindSocket(zmq::socket_t *socket, const char *address)
{
    socket->bind(address);
}

void ConnectSocket(zmq::socket_t *socket, const char *address)
{
    socket->connect(address);
}

void DestroySocket(zmq::socket_t *socket)
{
    delete socket;
}

void Send(zmq::socket_t *socket, const uint8_t *data, size_t size)
{
    zmq::message_t msg(data, size);
    socket->send(msg, zmq::send_flags::none);
}

size_t Receive(zmq::socket_t *socket, uint8_t *buffer, size_t maxSize)
{
    zmq::message_t msg;
    auto result = socket->recv(msg, zmq::recv_flags::none);
    if (result.has_value()) {
        memcpy(buffer, msg.data(), result.value());
        return result.value();
    }
    return 0;
}

void AddMesh(usd::StageProxy *proxy, const char *assetPath, const char *primPath)
{
    proxy->addMesh(assetPath, primPath);
}
