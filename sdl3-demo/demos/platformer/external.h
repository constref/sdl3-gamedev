#pragma once

#include <engineapi.h>
#include <stdint.h>
#include <tooling/engineworker.h>

#ifdef __cplusplus
extern "C" {
#endif

class EngineWorker;

int ENGINE_API StartAppStandalone();
ENGINE_API EngineWorker *StartAppTooling(int logW, int logH, int width, int height);
void ENGINE_API StartWorker(EngineWorker *worker, InitCallback callback);
uint64_t ENGINE_API GetSharedRenderTarget(EngineWorker *worker);

#ifdef __cplusplus
}
#endif
