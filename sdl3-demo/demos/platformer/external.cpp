#include "external.h"

#include "platformer.h"
#include <engine.h>
//#include <tooling/engineworker.h>

int StartAppStandalone()
{
	Engine engine(std::make_unique<Platformer>());
	if (!engine.initialize(512, 288))
	{
		return 1;
	}
	engine.run();

	return 0;
}

//extern "C" __declspec(dllexport) EngineWorker * __stdcall CreateEngine(int x, int y, int width, int height)
//{
//    return new EngineWorker<Platformer>(x, y, width, height);
//}

int StartAppTooling()
{
	Engine engine(std::make_unique<Platformer>());
	if (!engine.initialize(512, 288))
	{
		return 1;
	}
	engine.run();
	//EngineInterface<Platformer> interface;
	//interface.initialize(0, 512, 288);
	//interface.start();
	return 0;
}
