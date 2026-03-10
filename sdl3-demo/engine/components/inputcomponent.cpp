#include "inputcomponent.h"
#include <glm/glm.hpp>

#include <node.h>
#include <framecontext.h>
#include <inputstate.h>
#include <logger.h>
#include <messaging/commands.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>

InputComponent::InputComponent(Node &owner) : Component(owner)
{
	direction = { 0, 0, 0 };
}
