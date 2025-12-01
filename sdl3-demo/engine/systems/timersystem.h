#pragma once

#include <vector>
#include <systems/system.h>

class Timer;

struct TimerEntry
{
	NodeHandle owner;
	Timer &timer;
};

class TimerSystem : public System<FrameStage::Start>
{
	std::vector<TimerEntry> timers;

public:
	TimerSystem(Services &services);
	void update(Node &node);
};