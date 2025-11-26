#include <systems/timersystem.h>
#include <messaging/events.h>

TimerSystem::TimerSystem(Services &services) : System(services)
{
}

void TimerSystem::update(Node &node)
{
	// run times and generate events
	for (TimerEntry &entry : timers)
	{
		if (int timeouts = entry.timer.step(FrameContext::dt()); timeouts > 0)
		{
			services.eventQueue().enqueue<TimerTimeoutEvent>(entry.owner, 0, timeouts);
		}
	}
}
