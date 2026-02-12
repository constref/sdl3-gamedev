#include "engineinterface.h"
#include "engineworker.h"

//EngineInterface::EngineInterface()
//{
//	worker = nullptr;
//}
//
//bool EngineInterface::initialize(HWND hWnd, int width, int height)
//{
//	//if (!app)
//	//{
//	//	app = new ShapesDemo();
//	//	return app->initialize(hWnd, width, height, false, false);
//	//}
//	return false;
//}
//
//void EngineInterface::startWorkerThread()
//{
//	//worker = new EngineWorker(*app);
//	//worker->exec();
//}
//
//void EngineInterface::resize(int width, int height)
//{
//	//worker->enqueueEvent(ResizeEvent(width, height));
//}
//
//void EngineInterface::shutdown()
//{
//	// shutdown worker thread
//	//worker->enqueueEvent(CloseEvent());
//	//worker->stop();
//
//	//// cleanup renderer stuff
//	//if (app)
//	//{
//	//	app->shutdown();
//	//	delete app;
//	//	app = nullptr;
//	//}
//}
//
//void EngineInterface::onMouseMove(int x, int y)
//{
//	//app->onMouseMove(x, y);
//}
//
//void EngineInterface::onMouseButtonPressed(int buttonIndex)
//{
//	//app->onMouseButtonDown(buttonIndex);
//}
//
//void EngineInterface::onMouseButtonReleased(int buttonIndex)
//{
//	//app->onMouseButtonUp(buttonIndex);
//}
