#include "inputcomponent.h"
#include <glm/glm.hpp>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../inputstate.h"
#include "../events.h"
#include "../coresubjects.h"

InputComponent::InputComponent(GameObject &owner) : Component(owner)
{
	direction = 0;
}

void InputComponent::update(const FrameContext &ctx)
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
	if (ctx.input.keys[SDL_SCANCODE_K])
	{
		emit(ctx, static_cast<int>(Events::jump));
	}

	if (direction)
	{
		emit(ctx, static_cast<int>(Events::run));
	}

	directionSubject.notify(direction);
}

void InputComponent::onAttached(SubjectRegistry &registry)
{
	registry.registerSubject(CoreSubjects::DIRECTION, &directionSubject);
}
