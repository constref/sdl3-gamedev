#pragma once

#include <engineapi.h>
#include <stdint.h>
#include <tooling/engineworker.h>
#include <tooling/callbacks.h>

#ifdef __cplusplus
extern "C" {
#endif

class EngineWorker;

int ENGINE_API StartAppStandalone();
ENGINE_API EngineWorker *StartAppTooling(int logW, int logH, int width, int height);
void ENGINE_API StartWorker(EngineWorker *worker, InitCallback callback);
ExportedResources ENGINE_API GetSharedRenderTarget(EngineWorker *worker, int frameIndex);
intptr_t ENGINE_API ExportWorkCompleteSemaphore(EngineWorker *worker, int index);
intptr_t ENGINE_API ExportImageReadySemaphore(EngineWorker *worker, int index);

void ENGINE_API OnKeyUp(EngineWorker *worker, uint16_t scancode);
void ENGINE_API OnKeyDown(EngineWorker *worker, uint16_t scancode);

void ENGINE_API SetEventCallback(EngineWorker *worker, EventReceived callback);

#ifdef __cplusplus
}
#endif
