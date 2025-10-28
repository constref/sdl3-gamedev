#pragma once

#include <vector>
#include <memory>
#include <cassert>

#include <world.h>
#include <messaging/event.h>

struct QueuedEvent
{
	using HandlerFn = void(*)(Node &, const EventBase &);
	NodeHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch;
};

class EventQueue
{
	std::array<std::vector<QueuedEvent>, static_cast<size_t>(ComponentStage::SIZE)> queues;
	std::array<std::pair<size_t, size_t>, static_cast<size_t>(ComponentStage::SIZE)> indices; // read,write pairs

	EventQueue()
	{
		for (int i = 0; i < static_cast<size_t>(ComponentStage::SIZE); ++i)
		{
			// TODO: Reduce mem footprint here
			queues[i].resize(5000);
			indices[i].first = 0;
			indices[i].second = 0;
		}
	}

public:
	static EventQueue &get()
	{
		static EventQueue instance;
		return instance;
	}

	auto &getQueue(ComponentStage stage)
	{
		return queues[static_cast<size_t>(stage)];
	}
	auto &getIndices(ComponentStage stage)
	{
		return indices[static_cast<size_t>(stage)];
	}

	template<typename EventType, typename... Args>
	void enqueue(NodeHandle target, ComponentStage stage, Args&&... args)
	{
		auto &queue = getQueue(stage);
		auto &indices = getIndices(stage);
		queue[indices.second++] = QueuedEvent 
		{
			.target = target,
			.event = std::make_unique<EventType>(std::forward<Args>(args)...),
			.dispatch = [](Node &obj, const EventBase &e)
			{
				obj.notify<const EventType &>(static_cast<const EventType &>(e));
			}
		};
	}

	void dispatch(ComponentStage stage)
	{
		auto &queue = getQueue(stage);
		auto &indices = getIndices(stage);

		while (indices.first < indices.second)
		{
			QueuedEvent &nextItem = queue[indices.first++];
			nextItem.dispatch(World::get().getNode(nextItem.target), *nextItem.event);
		}
	}

	size_t getCount(ComponentStage stage)
	{
		auto &indices = getIndices(stage);
		return indices.second;
	}
};