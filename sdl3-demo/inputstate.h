#pragma once
#include <SDL3/SDL.h>

struct InputState
{
	bool keys[SDL_SCANCODE_COUNT]{ false };
};