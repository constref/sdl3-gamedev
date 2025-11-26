#pragma once

#include <vector>
#include <memory>
#include <cassert>

#include <world.h>
#include <messaging/event.h>
#include <framecontext.h>
#include <logger.h>


struct QueuedEvent
{
	using HandlerFn = bool(*)(EventDispatcher &, NodeHandle, const EventBase &);
	NodeHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch = nullptr;
	double triggerTime = 0;
	bool processed = false;
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
	auto &getQueue()
	{
		return queue;
	}

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
				if (numHandlers)
				{
					Logger::info(static_cast<const EventType *>(&e), std::format("Frame #{} : Dispatched to {} handlers.", FrameContext::global().frameNumber, numHandlers));
				}
				return numHandlers > 0;
			},
			.triggerTime = FrameContext::gt() + delay,
			.processed = false
		};
	}

	void dispatch()
	{
		size_t currRIdx = rIdx;
		size_t numEvents = wIdx - rIdx;
		size_t currWIdx = wIdx;
		int numHandled = 0;
		while (currRIdx < currWIdx)
		{
			QueuedEvent &item = queue[currRIdx++];
			if (!item.processed && FrameContext::gt() >= item.triggerTime)
			{
				// item processed if it has handlers for the current frame stage
				if (item.dispatch(dispatcher, item.target, *item.event))
				{
					item.processed = true;
					numHandled++;
				}
			}
		}
		if (numHandled == numEvents) rIdx = currRIdx;
	}

	size_t getCount()
	{
		return wIdx;
	}
};
