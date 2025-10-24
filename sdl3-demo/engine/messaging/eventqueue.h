#pragma once

#include <vector>
#include <memory>
#include <cassert>

#include <world.h>

class EventBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename EventType>
class Event : public EventBase
{
public:
	constexpr static int index()
	{
		assert(nextIndex < 1000 && "Exceeded maximum event index limit");
		static int idx = nextIndex++;
		return idx;
	}
};

struct QueuedEvent
{
	using HandlerFn = void(*)(GameObject &, const EventBase &);
	GHandle target;
	std::unique_ptr<EventBase> event;
	HandlerFn dispatch;
};

class EventQueue
{
	std::vector<QueuedEvent> queue;

	size_t readIdx;
	size_t writeIdx;

	EventQueue()
	{
		readIdx = 0;
		writeIdx = 0;
		queue.resize(5000);
	}

public:
	static EventQueue &get()
	{
		static EventQueue instance;
		return instance;
	}

	template<typename EventType, typename... Args>
	void enqueue(GHandle target, Args&&... args)
	{
		queue[writeIdx++] = QueuedEvent 
		{
			.target = target,
			.event = std::make_unique<EventType>(std::forward<Args>(args)...),
			.dispatch = [](GameObject &obj, const EventBase &e)
			{
				obj.notify<const EventType &>(static_cast<const EventType &>(e));
			}
		};
	}

	void dispatch(ComponentStage stage)
	{
		while (readIdx < writeIdx)
		{
			QueuedEvent &nextItem = queue[readIdx++];
			nextItem.dispatch(World::get().getObject(nextItem.target), *nextItem.event);
		}
	}

	size_t getCount() const
	{
		return writeIdx;
	}
};