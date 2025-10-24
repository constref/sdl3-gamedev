#include "inputcomponent.h"
#include <glm/glm.hpp>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../inputstate.h"
#include "../messaging/datapumps.h"

#include <messaging/events.h>

InputComponent::InputComponent(GameObject &owner, GHandle ownerHandle) : Component(owner, ComponentStage::Input)
{
	direction = 0;
	this->ownerHandle = ownerHandle;
}

void InputComponent::onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<InputComponent, KeyboardEvent>(this);

	// ensure global input state knows this object has input focus
	// TODO: Need a more robust solution to support 2+ players
	InputState::get().setFocus(ownerHandle);
}

void InputComponent::update(const FrameContext &ctx)
{

	//KeyEvent keyEvent;
	//while (ctx.input.popEvent(keyEvent))
	//{
	//	switch (keyEvent.scancode)
	//	{
	//		case SDL_SCANCODE_K:
	//		{
	//			if (keyEvent.pressed)
	//			{
	//				owner.sendMessage(JumpDPump{});
	//			}
	//			break;
	//		}
	//		case SDL_SCANCODE_J:
	//		{
	//			if (keyEvent.pressed)
	//			{
	//				owner.sendMessage(ShootStartDPump{});
	//			}
	//			else
	//			{
	//				owner.sendMessage(ShootEndDPump{});
	//			}
	//			break;
	//		}
	//	}
	//}

	//owner.sendMessage(DirectionDPump{ direction });
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
	owner.sendMessage(DirectionDPump{ direction });

	switch (event.scancode)
	{
		case SDL_SCANCODE_K:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				owner.sendMessage(JumpDPump{});
			}
			break;
		}
		case SDL_SCANCODE_J:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				owner.sendMessage(ShootStartDPump{});
			}
			else
			{
				owner.sendMessage(ShootEndDPump{});
			}
			break;
		}
	}
}
