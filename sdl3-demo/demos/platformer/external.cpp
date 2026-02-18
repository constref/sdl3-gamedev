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

ExportedResources GetSharedRenderTarget(EngineWorker *worker, int frameIndex)
{
    return worker->getEngine().getRenderer()->getSharedRenderTarget(frameIndex);
}

intptr_t ENGINE_API ExportWorkCompleteSemaphore(EngineWorker *worker, int index)
{
	return worker->getEngine().getRenderer()->exportWorkCompleteSemaphore(index);
}
intptr_t ENGINE_API ExportImageReadySemaphore(EngineWorker *worker, int index)
{
	return worker->getEngine().getRenderer()->exportImageReadySemaphore(index);
}

void ENGINE_API OnKeyUp(EngineWorker *worker, uint16_t scancode)
{
	worker->pushEvent(KeyUp{ .scancode = scancode });
}

void ENGINE_API OnKeyDown(EngineWorker *worker, uint16_t scancode)
{
	worker->pushEvent(KeyDown{ .scancode = scancode });
}

void ENGINE_API SetEventCallback(EngineWorker *worker, EventReceived callback)
{
	Logger::logHandler = [callback](std::string message) {
		callback(message.c_str());
	};
}
