#pragma once

#include <engine_generated.h>
#include <thread>
#include <engine.h>
#include <memory>
#include <containers/atomicringbuffer.h>
#include "platformevents.h"

class EngineWorker
{
	std::unique_ptr<Engine> m_engine;
	bool m_running;
	AtomicRingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread m_engineThread;

public:
	EngineWorker(std::unique_ptr<Application> app);
	~EngineWorker();

	void start();
	void processEvents();
	void pushEvent(const PlatformEvent &event);
	Engine &getEngine();
};
