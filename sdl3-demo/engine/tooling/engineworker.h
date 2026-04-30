#pragma once

#include "platformevents.h"
#include <thread>
#include <engine.h>
#include <memory>
#include <containers/atomicringbuffer.h>

class EngineWorker
{
	std::unique_ptr<Engine> m_engine;
	int editorPID;
	std::string url;

	bool m_listening;
	bool m_running;
	AtomicRingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread publisherThread;
	std::thread m_engineThread;
	std::thread m_repThread;
	std::thread m_subThread;

public:
	EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string &url);
	~EngineWorker();

	void start();
	void processEvents();
	void pushEvent(const PlatformEvent &event);
	Engine &getEngine();
};
