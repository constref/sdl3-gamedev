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
	direction = 0;
	this->ownerHandle = ownerHandle;

	owner.getEventDispatcher().registerHandler<KeyboardEvent>(this);

	// ensure global input state knows this object has input focus
	// TODO: Need a more robust solution to support 2+ players
	InputState::get().setFocus(ownerHandle);
}

void InputComponent::onEvent(const KeyboardEvent &event)
{
	auto &state = InputState::get();
	state.setKeyState(event.scancode, event.state == KeyboardEvent::State::down);

	float direction = 0;
	if (state.isKeyPressed(SDL_SCANCODE_A))
	{
		direction += -1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_D))
	{
		direction += 1;
	}
	owner.sendCommand(UpdateDirectionCommand{ direction });

	switch (event.scancode)
	{
		case SDL_SCANCODE_K:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				EventQueue::get().enqueue<JumpEvent>(owner.getHandle(), 0);
			}
			break;
		}
		case SDL_SCANCODE_J:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				EventQueue::get().enqueue<ShootBeginEvent>(owner.getHandle(), 0);
			}
			else
			{
				EventQueue::get().enqueue<ShootEndEvent>(owner.getHandle(), 0);
			}
			break;
		}
	}
}
