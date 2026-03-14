#pragma once

#include <wtypes.h>
#include <thread>
#include <array>
#include <stdint.h>

#include <application.h>
#include <engine.h>
#include <memory>
#include <containers/atomicringbuffer.h>
#include <messaging/events.h>
#include "platformevents.h"

class EngineWorker
{
	std::unique_ptr<Engine> engine;
	int editorPID;
	std::string editorUrl;
	int logW, logH, width, height;

	bool shouldRun;
	AtomicRingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread publisherThread;
	std::thread pullThread;

public:
	EngineWorker(std::unique_ptr<Engine> engine, int editorPID, const std::string &handshakeUrl, int logW, int logH, int width, int height);
	~EngineWorker();

	void start();
	void processEvents();
	void pushEvent(const PlatformEvent &event);
	Engine &getEngine();
};
