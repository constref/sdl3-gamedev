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

class Interop;

class EngineWorker
{
	std::unique_ptr<Engine> engine;
	int editorPID;
	int logW, logH, width, height;

	bool shouldRun;
	AtomicRingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread publisherThread;
	Interop *interop;

public:
	EngineWorker(std::unique_ptr<Engine> engine, int editorPID, int logW, int logH, int width, int height);
	~EngineWorker();

	void start();
	void processEvents();
	void pushEvent(const PlatformEvent &event);
	Engine &getEngine();
	void processInteropMessage(uint32_t msgType);
};
