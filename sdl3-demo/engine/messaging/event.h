#pragma once

#include <cassert>

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
