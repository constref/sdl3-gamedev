#pragma once
#include <string>

#include "usdprocessor.h"
#include <zmq.hpp>

namespace usd
{
    class UsdProcessor;
};

#define DLL_EXPORT
#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" {
using PrimList = std::vector<usd::Prim>;

DLL_API usd::UsdProcessor* CreateStage(const char *path);
DLL_API usd::UsdProcessor* OpenStage(const char *path);
DLL_API void SaveStage(usd::UsdProcessor *proc);
DLL_API void DestroyUsdProcessor(usd::UsdProcessor *proc);

DLL_API PrimList* BuildPrimList(usd::UsdProcessor *proc, bool useDefaultPrim);
DLL_API usd::Prim* GetPrimListData(PrimList *list, uint32_t *outSize);
DLL_API void DestroyPrimList(PrimList *list);

DLL_API zmq::context_t* CreateContext();
DLL_API void DestroyContext(zmq::context_t *context);
DLL_API zmq::socket_t* CreateRequestSocket(zmq::context_t *context);
DLL_API void BindSocket(zmq::socket_t *socket, const char *address);
DLL_API void ConnectSocket(zmq::socket_t *socket, const char *address);
DLL_API void DestroySocket(zmq::socket_t *socket);
DLL_API void Send(zmq::socket_t *socket, const uint8_t *data, size_t size);
DLL_API size_t Receive(zmq::socket_t *socket, uint8_t *buffer, size_t maxSize);
}
