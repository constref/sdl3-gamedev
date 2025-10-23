#include "inputcomponent.h"
#include <glm/glm.hpp>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../inputstate.h"
#include "../messaging/datapumps.h"

InputComponent::InputComponent(GameObject &owner) : Component(owner, ComponentStage::Input)
{
	direction = 0;
}

void InputComponent::update(const FrameContext &ctx)
{
	float direction = 0;
	if (ctx.input.isKeyPressed(SDL_SCANCODE_A))
	{
		direction += -1;
	}
	if (ctx.input.isKeyPressed(SDL_SCANCODE_D))
	{
		direction += 1;
	}

	KeyEvent keyEvent;
	while (ctx.input.popEvent(keyEvent))
	{
		switch (keyEvent.scancode)
		{
			case SDL_SCANCODE_K:
			{
				if (keyEvent.pressed)
				{
					owner.sendMessage(JumpDPump{});
				}
				break;
			}
			case SDL_SCANCODE_J:
			{
				if (keyEvent.pressed)
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

	owner.sendMessage(DirectionDPump{ direction });
}
