#pragma once

#include <wtypes.h>
#include <thread>
#include <array>

#include <application.h>
#include <engine.h>
#include <memory>
#include <tooling/callbacks.h>
#include "containers/ringbuffer.h"
#include "platformevents.h"

class EngineWorker
{
	std::unique_ptr<Engine> engine;
	int logW, logH, width, height;

	bool shouldRun;
	RingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread workThread;

public:
	EngineWorker(std::unique_ptr<Engine> engine, int logW, int logH, int width, int height) : engine(std::move(engine))
	{
		this->logW = logW;
		this->logH = logH;
		this->width = width;
		this->height = height;
		this->shouldRun = false;
	}

	~EngineWorker()
	{
		shouldRun = false;
		if (workThread.joinable())
		{
			workThread.join();
		}
	}

	void initialize(InitCallback callback)
	{
		shouldRun = true;
		if (!engine->initialize(logW, logH, width, height))
		{
			shouldRun = false;
		}

		if (shouldRun)
		{
			if (callback)
			{
				callback(engine->getRenderer()->getRenderInfo());
			}
		}
	}

	void start()
	{
		workThread = std::thread([this]()
		{
			while (shouldRun)
			{
				engine->step();
			}

			return 0;
		});
	}

	void processEvents()
	{
		//    PlatformEvent e;
		//    while (eventBuffer.get(e))
		//    {
		//        // handle external events
		//        if (std::holds_alternative<ApplicationEnteredBackground>(e))
		//        {
		//            app.pause();
		//        }
		//        else if (std::holds_alternative<ApplicationEnteredForeground>(e))
		//        {
		//            app.resume();
		//        }
		//        else if (std::holds_alternative<MouseMoveEvent>(e))
		//        {
		//            const MouseMoveEvent &mouseMoveEvent = std::get<MouseMoveEvent>(e);
		//            app.onMouseMove(mouseMoveEvent.x, mouseMoveEvent.y);
		//        }
		//        else if (std::holds_alternative<MouseButtonEvent>(e))
		//        {
		//            const MouseButtonEvent &mouseButtonEvent = std::get<MouseButtonEvent>(e);
		//            if (mouseButtonEvent.isDown)
		//            {
		//                app.onMouseButtonDown(mouseButtonEvent.buttonIndex);
		//            }
		//            else
		//            {
		//                app.onMouseButtonUp(mouseButtonEvent.buttonIndex);
		//            }
		//        }
		//        else if (std::holds_alternative<ResizeEvent>(e))
		//        {
		//            const ResizeEvent &event = std::get<ResizeEvent>(e);
		//            app.onResizeRenderer(event.x, event.y, event.width, event.height);
		//        }
		//        else if (std::holds_alternative<ExitEvent>(e))
		//        {
		//            shouldRun = false;
		//        }
		//    }
	}

	void pushEvent(const PlatformEvent &event)
	{
		eventBuffer.add(event);
	}

	Engine &getEngine()
	{
		return *engine;
	}
};
