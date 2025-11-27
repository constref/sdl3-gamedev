#include <messaging/eventqueue.h>

void EventQueue::dispatch()
{
	size_t currRIdx = rIdx; // currRIdx is what actually does reading, rIdx only moves when all events are completed
	size_t numEvents = wIdx - rIdx;
	size_t currWIdx = wIdx; // take a snapshot of wIdx to
	int eventsCompleted = 0;
	while (currRIdx < currWIdx)
	{
		QueuedEvent &item = queue[currRIdx++];
		if (item.remainingHandlers > 0 && FrameContext::gt() >= item.triggerTime)
		{
			item.remainingHandlers -= item.dispatch(dispatcher, item.target, *item.event);
			if (item.remainingHandlers == 0)
			{
				// all registered handlers have received this event
				eventsCompleted++;
			}
		}
	}
	// all outstanding events completed, move main read-index further now
	if (eventsCompleted == numEvents) rIdx = currRIdx;
}
