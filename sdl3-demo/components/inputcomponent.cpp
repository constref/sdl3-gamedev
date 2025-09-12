#include "inputcomponent.h"
#include <glm/glm.hpp>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../inputstate.h"

InputComponent::InputComponent() : Component()
{
}

void InputComponent::update(GameObject &owner, const FrameContext &ctx)
{
	float direction = 0;
	if (ctx.input.keys[SDL_SCANCODE_A])
	{
		direction += -1;
	}
	if (ctx.input.keys[SDL_SCANCODE_D])
	{
		direction += 1;
	}

	directionUpdate.notify(direction);
}
