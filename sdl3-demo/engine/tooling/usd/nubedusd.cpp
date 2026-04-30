#include "nubedusd.h"

// #include <pxr/usd/usd/stage.h>
// #include <pxr/usd/usd/prim.h>
// #include <pxr/usd/usdGeom/tokens.h>
// #include <pxr/usd/usd/notice.h>
#include "common.h"
#include "stageproxy.h"

using namespace usd;

StageProxy *CreateProject(const char *path, ObjectsChangedFunc objectsChangedCallback)
{
    return StageProxy::create(path, objectsChangedCallback);
}

StageProxy *OpenProject(const char *path, ObjectsChangedFunc objectsChangedCallback)
{
    return StageProxy::open(path, objectsChangedCallback);
}

void SaveProject(usd::StageProxy *proxy)
{
    proxy->save();
}

void DestroyProxy(usd::StageProxy *proxy)
{
    delete proxy;
}

usd::StageProxy *OpenStage(const char *path)
{
    return StageProxy::open(path, nullptr);
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

void AddMesh(usd::StageProxy *proxy, const char *assetId, const char *assetPath, const char *primPath)
{
    proxy->addMesh(assetId, assetPath, primPath);
    proxy->flushChanged();
}

void CreateBrush(StageProxy *proxy, const char *primPath)
{
    proxy->createBrush(primPath);
    proxy->flushChanged();
}

void PlaceBrush(StageProxy *proxy, const char *brushPath)
{
    proxy->placeBrush(brushPath);
    proxy->flushChanged();
}


void FlushChanges(StageProxy *proxy)
{
    proxy->flushChanged();
}

void Bake(usd::StageProxy *proxy, const char *nubPath)
{
}
