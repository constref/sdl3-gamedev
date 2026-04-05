#include "nubedusd.h"

#include <pxr/usd/usd/stage.h>
#include <tooling/usd/usdprocessor.h>

usd::UsdProcessor *CreateStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->createStage(path);
    return usdProc;
}

usd::UsdProcessor *OpenStage(const char *path)
{
    auto usdProc = new usd::UsdProcessor(nullptr);
    usdProc->openStage(path);
    return usdProc;
}

void SaveStage(usd::UsdProcessor *proc)
{
    proc->saveStage();
}

void DestroyUsdProcessor(usd::UsdProcessor *proc)
{
    delete proc;
}

PrimList *BuildPrimList(usd::UsdProcessor *proc, bool useDefaultPrim)
{
    PrimList *list = new PrimList();
    proc->flatten(*list, useDefaultPrim);
    return list;
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
    if (result.has_value())
    {
        memcpy(buffer, msg.data(), result.value());
        return result.value();
    }
    return 0;
}