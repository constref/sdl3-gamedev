#include "component.h"
#include "../gameobject.h"

void Component::emit(const FrameContext &ctx, int eventId)
{
	owner.notify(ctx, eventId);
}
