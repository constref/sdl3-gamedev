#include "external.h"

#include "platformer.h"
#include <engine.h>
#include <tooling/engineworker.h>


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

void StartAppTooling(int logW, int logH, int width, int height)
{
	EngineWorker *worker = new EngineWorker(std::make_unique<Engine>(std::make_unique<Platformer>()), logW, logH, width, height);
	worker->start();
}

ExportedResources GetSharedRenderTarget(EngineWorker *worker, int frameIndex)
{
    return worker->getEngine().getRenderer()->getSharedRenderTarget(frameIndex);
}

intptr_t ExportWorkCompleteSemaphore(EngineWorker *worker, int index)
{
	return worker->getEngine().getRenderer()->exportWorkCompleteSemaphore(index);
}
intptr_t ExportImageReadySemaphore(EngineWorker *worker, int index)
{
	return worker->getEngine().getRenderer()->exportImageReadySemaphore(index);
}

void OnKeyUp(EngineWorker *worker, uint16_t scancode)
{
	worker->pushEvent(KeyUp{ .scancode = scancode });
}

void OnKeyDown(EngineWorker *worker, uint16_t scancode)
{
	worker->pushEvent(KeyDown{ .scancode = scancode });
}

void SetEventCallback(EngineWorker *worker, EventReceived callback)
{
	Logger::logHandler = [callback](std::string message) {
		callback(message.c_str());
	};
}

void SendInteropMessage(EngineWorker *worker, int msgId)
{
	worker->processInteropMessage(msgId);
}
