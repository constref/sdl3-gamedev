#pragma once

#include <glm/glm.hpp>
#include "../state.h"

class GameObject;
struct FrameContext;

class Controller
{
	State *currentState;

protected:
	GameObject &owner;

public:
	Controller(GameObject &owner) : owner(owner)
	{
		currentState = nullptr;
	}
	virtual ~Controller() { }

	virtual void eventHandler(const FrameContext &ctx, int eventId)
	{
		if (currentState)
		{
			currentState->onEvent(owner, eventId);
		}
	}

	virtual void collisionHandler(GameObject &other, glm::vec2 overlap) = 0;
	void changeState(State *state)
	{
		if (currentState)
		{
			currentState->onExit(owner);
		}
		currentState = state;
		currentState->onEnter(owner);
	}
};