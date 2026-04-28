#pragma once
#include <string>

#include "usdprocessor.h"
#include <zmq.hpp>
#include "common.h"

namespace usd
{
    class StageProxy;
    class UsdProcessor;
    using PrimList = std::vector<usd::Prim>;
};

#define DLL_EXPORT
#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" {

DLL_API usd::StageProxy *CreateProject(const char *path, usd::ObjectsChangedFunc objectsChangedCallback);
DLL_API usd::StageProxy *OpenProject(const char *path, usd::ObjectsChangedFunc objectsChangedCallback);
DLL_API void SaveProject(usd::StageProxy *proxy);
DLL_API void DestroyProxy(usd::StageProxy *proxy);
DLL_API usd::StageProxy *OpenStage(const char *path);

DLL_API usd::PrimList *BuildPrimList(usd::StageProxy *proxy, bool useDefaultPrim);
DLL_API usd::Prim* GetPrimListData(usd::PrimList *list, uint32_t *outSize);
DLL_API void DestroyPrimList(usd::PrimList *list);

DLL_API void AddMesh(usd::StageProxy *proxy, const char *assetPath, const char *primPath);
DLL_API void CreateBrush(usd::StageProxy *proxy, const char *primPath);
DLL_API void PlaceBrush(usd::StageProxy *proxy, const char *brushPath);
DLL_API void FlushChanges(usd::StageProxy *proxy);

DLL_API void SaveStage(usd::UsdProcessor *proc);
DLL_API void DestroyUsdProcessor(usd::UsdProcessor *proc);

DLL_API zmq::context_t* CreateContext();
DLL_API void DestroyContext(zmq::context_t *context);
DLL_API zmq::socket_t* CreateSocket(zmq::context_t *context, int socketType);
DLL_API void BindSocket(zmq::socket_t *socket, const char *address);
DLL_API void ConnectSocket(zmq::socket_t *socket, const char *address);
DLL_API void DestroySocket(zmq::socket_t *socket);
DLL_API void Send(zmq::socket_t *socket, const uint8_t *data, size_t size);
DLL_API size_t Receive(zmq::socket_t *socket, uint8_t *buffer, size_t maxSize);
}
