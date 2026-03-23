#pragma once

#include <thread>
#include <memory>
#include <containers/atomicringbuffer.h>
#include <windowhandle.h>
#include "platformevents.h"


class Engine;
class Application;

class EngineWorker
{
	std::unique_ptr<Engine> m_engine;
	bool m_running;
	AtomicRingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread m_engineThread;

public:
	EngineWorker(std::unique_ptr<Application> app);
	~EngineWorker();

	void start(WindowHandle hWnd, int width, int height);
	void stop();
	void processEvents();
	void pushEvent(const PlatformEvent &event);
	Engine &getEngine();
};
