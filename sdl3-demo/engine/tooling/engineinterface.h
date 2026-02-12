#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include <application.h>
#include <tooling/engineworker.h>

template<Application App>
class ENGINE_API EngineInterface
{
	EngineWorker<App> *worker;

public:
	EngineInterface()
	{
		worker = nullptr;
	}
	~EngineInterface()
	{
		if (worker)
		{
			delete worker;
		}
	}

	bool initialize(HWND hWnd, int width, int height)
	{
		if (!worker)
		{
			worker = new EngineWorker<App>(hWnd, 0, 0, width, height);
			worker->start(nullptr);
		}
		return true;
	}
	void resize(int width, int height)
	{
	}
	void startWorkerThread()
	{

	}
	void onMouseMove(int x, int y)
	{

	}
	void onMouseButtonPressed(int buttonIndex)
	{

	}
	void onMouseButtonReleased(int buttonIndex)
	{

	}
	void shutdown()
	{

	}
};
