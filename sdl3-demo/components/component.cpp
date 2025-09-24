#include "component.h"
#include "../gameobject.h"

void Component::emit(int eventId)
{
	owner.notify(eventId);
}
