#pragma once

#include <vector>
#include <memory>
#include <cassert>

#include <world.h>
#include <messaging/event.h>
#include <framecontext.h>

struct QueuedEvent
{
	using HandlerFn = void(*)(Node &, const EventBase &);
	NodeHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch = nullptr;
	double triggerTime = 0;
	bool processed = false;
};

struct QueuedEvent2
{
	using HandlerFn = void(*)(EventDispatcher2 &dispatcher, NodeHandle handle, const EventBase &);
	NodeHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch = nullptr;
	double triggerTime = 0;
	bool processed = false;
};

class EventQueue
{
	std::array<std::vector<QueuedEvent2>, static_cast<size_t>(FrameStage::StageCount)> queues;
	std::array<std::pair<size_t, size_t>, static_cast<size_t>(FrameStage::StageCount)> indices; // read,write pairs

public:
	EventQueue()
	{
		for (int i = 0; i < static_cast<size_t>(FrameStage::StageCount); ++i)
		{
			// TODO: Reduce mem footprint here
			queues[i].resize(5000);
			indices[i].first = 0;
			indices[i].second = 0;
			queues[i].resize(5000);
		}
	}

	EventDispatcher2 dispatcher;

	auto &getQueue(FrameStage stage)
	{
		return queues[static_cast<size_t>(stage)];
	}
	auto &getIndices(FrameStage stage)
	{
		return indices[static_cast<size_t>(stage)];
	}

	template<typename EventType, typename... Args>
	void enqueue(NodeHandle target, float delay, Args&&... args)
	{
		auto &queue = queues[static_cast<size_t>(EventType::stage)];
		auto &indices = getIndices(EventType::stage);

		size_t wIdx = indices.second++;
		queue[wIdx] = QueuedEvent2
		{
			.target = target,
			.event = std::make_unique<EventType>(std::forward<Args>(args)...),
			.dispatch = [](EventDispatcher2 &dispatcher, NodeHandle target, const EventBase &e)
			{
				dispatcher.send2<EventType>(target, static_cast<const EventType &>(e));
			},
			.triggerTime = FrameContext::gt() + delay,
			.processed = false
		};
	}

	void dispatch(FrameStage stage)
	{
		auto &queue = queues[static_cast<size_t>(stage)];
		auto &indices = getIndices(stage);

		size_t rIdx = indices.first;
		size_t numEvents = indices.second - indices.first;
		int numDispatched = 0;
		while (rIdx < indices.second)
		{
			QueuedEvent2 &item = queue[rIdx++];
			if (!item.processed && item.triggerTime <= FrameContext::gt())
			{
				item.dispatch(dispatcher, item.target, *item.event);
				item.processed = true;
				numDispatched++;
			}
		}
		if (numEvents == numDispatched)
		{
			indices.first = rIdx;
		}
	}

	size_t getCount(FrameStage stage)
	{
		auto &indices = getIndices(stage);
		return indices.second;
	}
};
