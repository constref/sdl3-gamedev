#pragma once

#include "platformevents.h"
#include <thread>
#include <engine.h>
#include <memory>
#include <containers/atomicringbuffer.h>

namespace usd { class UsdProcessor; }

class EngineWorker
{
	std::unique_ptr<Engine> m_engine;
	std::unique_ptr<usd::UsdProcessor> m_usdProcessor;
	int editorPID;
	std::string url;

	bool m_listening;
	bool m_running;
	AtomicRingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread publisherThread;
	std::thread m_engineThread;
	std::thread m_repThread;
	std::thread m_pullThread;

public:
	EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string &url);
	~EngineWorker();

	void start();
	void processEvents();
	void pushEvent(const PlatformEvent &event);
	Engine &getEngine();
};
