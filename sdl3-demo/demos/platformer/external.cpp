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

void StartAppTooling(int editorPID, int logW, int logH, int width, int height)
{
	EngineWorker *worker = new EngineWorker(std::make_unique<Engine>(std::make_unique<Platformer>()), editorPID, logW, logH, width, height);
	worker->start();
}
