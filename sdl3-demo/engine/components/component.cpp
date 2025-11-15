#include "component.h"
#include <node.h>
#include <messaging/commanddispatcher.h>
#include <messaging/eventdispatcher.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <logger.h>
#include <framecontext.h>

Component::~Component()
{
}

void Component::earlyUpdate()
{
	// run times and generate events
	//for (Timer *t : timers)
	//{
	//	if (int timeouts = t->step(FrameContext::dt()); timeouts > 0)
	//	{
	//		services.eventQueue().enqueue<TimerOnTimeout>(owner.getHandle(), 0, timeouts);
	//	}
	//}
}

void Component::addTimer(Timer &timer)
{
	auto itr = std::find(timers.begin(), timers.end(), &timer);
	if (itr == timers.end())
	{
		timers.push_back(&timer);
	}
}

void Component::removeTimer(const Timer &timer)
{
	auto itr = std::find(timers.begin(), timers.end(), &timer);
	if (itr != timers.end())
	{
		timers.erase(itr);
	}
	else
	{
		Logger::warn(this, "Tried to remove non-existent timer.");
	}
}
