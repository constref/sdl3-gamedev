#pragma once

#include <messaging/eventqueue.h>
#include <SDL3/SDL.h>

struct KeyboardEvent : public Event<KeyboardEvent>
{
	enum class State
	{
		down,
		up
	};

	SDL_Scancode scancode;
	State state;

	KeyboardEvent(SDL_Scancode scancode, State state) :
		scancode(scancode), state(state)
	{
	}
};
