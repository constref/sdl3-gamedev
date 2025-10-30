#include "component.h"
#include <node.h>
#include <messaging/commanddispatcher.h>
#include <messaging/eventdispatcher.h>

Component::~Component()
{
	owner.getCommandDispatcher().unregisterHandler(this);
	owner.getEventDispatcher().unregisterHandler(this);
}
