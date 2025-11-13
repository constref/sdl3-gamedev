#include "inputcomponent.h"
#include <glm/glm.hpp>

#include <node.h>
#include <framecontext.h>
#include <inputstate.h>
#include <logger.h>
#include <messaging/commands.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>

InputComponent::InputComponent(Node &owner, NodeHandle ownerHandle) : Component(owner, FrameStage::Input)
{
	this->ownerHandle = ownerHandle;
	direction = { 0, 0 };

	// ensure global input state knows this object has input focus
	// TODO: Need a more robust solution to support 2+ players
	InputState::get().setFocus(ownerHandle);
}
