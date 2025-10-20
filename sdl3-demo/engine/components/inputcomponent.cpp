#include "inputcomponent.h"
#include <glm/glm.hpp>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../inputstate.h"
#include "../messaging/messages.h"

InputComponent::InputComponent(GameObject &owner) : Component(owner)
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
					owner.sendMessage(JumpMessage{});
				}
				break;
			}
		}
	}

	owner.sendMessage(DirectionMessage{ direction });
}
