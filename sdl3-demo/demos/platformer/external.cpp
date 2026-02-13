#include "external.h"

#include "platformer.h"
#include <engine.h>

int StartAppStandalone()
{
	Engine engine(std::make_unique<Platformer>());
	if (!engine.initialize(512, 288, 1920, 1080))
	{
		return 1;
	}
	engine.run();

	return 0;
}

EngineWorker *StartAppTooling(int logW, int logH, int width, int height)
{
	EngineWorker *worker = new EngineWorker(std::make_unique<Engine>(std::make_unique<Platformer>()), logW, logH, width, height);
	return worker;
}

void StartWorker(EngineWorker *worker, InitCallback callback)
{
	worker->start(callback);
}

uint64_t GetSharedRenderTarget(EngineWorker *worker)
{
    return worker->getSharedRenderTarget();
}
