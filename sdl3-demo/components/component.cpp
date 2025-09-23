#include "component.h"
#include "../gameobject.h"

void Component::emit(int eventId)
{
	auto o = owner.lock();
	if (o)
	{
		o->notify(eventId);
	}
}
