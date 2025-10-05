#include "inputcomponent.h"
#include <glm/glm.hpp>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../inputstate.h"
#include "../messaging/events.h"
#include "../messaging/coresubjects.h"
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
					//owner.getCommandDispatch().dispatch(Command{ .id = Commands::Jump });
				}
				break;
			}
		}
	}

	directionSubject.notify(direction);
}

void InputComponent::onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch)
{
	registry.registerSubject(CoreSubjects::DIRECTION, &directionSubject);
}
