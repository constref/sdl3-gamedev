#pragma once

#include <wtypes.h>
#include <thread>
#include <array>

#include <application.h>
#include <engine.h>
#include "containers/ringbuffer.h"
#include "platformevents.h"

typedef void(__stdcall InitReadCallback)(bool);

template<Application App>
class EngineWorker
{
	Engine<App> engine;
	int x, y, width, height;
	HWND hWnd;
	bool shouldRun;
	RingBuffer<PlatformEvent, 64> eventBuffer;
	std::thread workThread;
	uint64_t *sharedRenderTarget;

public:
	EngineWorker(HWND hWnd, int x, int y, int width, int height)
	{
		this->hWnd = hWnd;
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->shouldRun = false;
		this->sharedRenderTarget = nullptr;
	}

	~EngineWorker()
	{
		shouldRun = false;
		if (workThread.joinable())
		{
			workThread.join();
		}
	}


	void start(InitReadCallback callback)
	{
		workThread = std::thread([this, callback]()
		{
			shouldRun = true;
			if (!engine.initialize(512, 288))
			{
				shouldRun = false;
				return 1;
			}

			while (shouldRun)
			{
				engine.step();
			}
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
	uint64_t *getSharedRenderTarget() const
	{
		return sharedRenderTarget;
	}
};
