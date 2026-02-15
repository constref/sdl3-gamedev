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
void ENGINE_API InitializeEngine(EngineWorker *worker, InitCallback callback);
void ENGINE_API StartWorker(EngineWorker *worker);
ExportedResources ENGINE_API GetSharedRenderTarget(EngineWorker *worker, int frameIndex);

#ifdef __cplusplus
}
#endif
