#pragma once

#include <cassert>
#include <framestage.h>

class EventBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename EventType, FrameStage CompStage = FrameStage::Gameplay>
class Event : public EventBase
{
public:
	constexpr static int typeIndex()
	{
		assert(nextIndex < 1000 && "Exceeded maximum event index limit");
		static int idx = nextIndex++;
		return idx;
	}

	const static inline FrameStage stage = CompStage;
};
