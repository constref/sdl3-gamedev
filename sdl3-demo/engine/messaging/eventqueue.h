#pragma once

#include <vector>
#include <memory>
#include <cassert>

#include <world.h>
#include <messaging/event.h>
#include <framecontext.h>

struct QueuedEvent
{
	using HandlerFn = size_t(*)(EventDispatcher &, NodeHandle, const EventBase &);
	NodeHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch = nullptr;
	double triggerTime = 0;
	size_t remainingHandlers = 0;
};

class EventQueue
{
	std::vector<QueuedEvent> queue;
	size_t rIdx, wIdx;

public:
	EventQueue()
	{
		queue.resize(5000);
		rIdx = 0;
		wIdx = 0;
	}

	EventDispatcher dispatcher;

	template<typename EventType, typename... Args>
	void enqueue(NodeHandle target, float delay, Args&&... args)
	{
		queue[wIdx++] = QueuedEvent
		{
			.target = target,
			.event = std::make_unique<EventType>(std::forward<Args>(args)...),
			.dispatch = [](EventDispatcher &dispatcher, NodeHandle target, const EventBase &e)
			{
				size_t numHandlers = dispatcher.send<EventType>(target, static_cast<const EventType &>(e), FrameContext::currentStage());
				return numHandlers;
			},
			.triggerTime = FrameContext::gt() + delay,
			.remainingHandlers = dispatcher.getHandlerCount<EventType>()
		};
	}

	void dispatch();

	size_t getCount()
	{
		return wIdx;
	}
};
