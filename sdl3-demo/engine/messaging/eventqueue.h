#pragma once

#include <vector>
#include <memory>
#include <cassert>

#include <world.h>
#include <messaging/event.h>
#include <framecontext.h>
#include <systems/system.h>

struct QueuedEvent
{
	using HandlerFn = void(*)(NodeHandle handle, const EventBase &);
	NodeHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch = nullptr;
	double triggerTime = 0;
	bool processed = false;
};

class EventQueue
{
	// each node has its own mailbox

	std::array<std::vector<QueuedEvent>, static_cast<size_t>(FrameStage::StageCount)> queues;
	std::array<std::pair<size_t, size_t>, static_cast<size_t>(FrameStage::StageCount)> indices; // read,write pairs

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

public:
	EventDispatcher2 dispatcher;

	static EventQueue &get()
	{
		static EventQueue instance;
		return instance;
	}

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
		queue[wIdx] = QueuedEvent
		{
			.target = target,
			.event = std::make_unique<EventType>(std::forward<Args>(args)...),
			.dispatch = [](NodeHandle target, const EventBase &e)
			{
				EventQueue::get().dispatcher.send2<EventType>(target, static_cast<const EventType &>(e));
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
			QueuedEvent &item = queue[rIdx++];
			if (!item.processed && item.triggerTime <= FrameContext::gt())
			{
				item.dispatch(item.target, *item.event);
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
