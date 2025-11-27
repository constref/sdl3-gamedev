#include <messaging/eventqueue.h>

void EventQueue::dispatch()
{
	size_t currRIdx = rIdx;
	size_t numEvents = wIdx - rIdx;
	size_t currWIdx = wIdx;
	int eventsCompleted = 0;
	while (currRIdx < currWIdx)
	{
		QueuedEvent &item = queue[currRIdx++];
		if (item.remainingHandlers > 0 && FrameContext::gt() >= item.triggerTime)
		{
			// item processed if it has handlers for the current frame stage
			item.remainingHandlers -= item.dispatch(dispatcher, item.target, *item.event);
			if (item.remainingHandlers == 0)
			{
				eventsCompleted++;
			}
		}
	}
	if (eventsCompleted == numEvents) rIdx = currRIdx;
}
