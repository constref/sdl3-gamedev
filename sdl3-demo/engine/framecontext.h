#pragma once

#include <ghandle.h>

struct SDLState;
struct Resources;
class InputState;

struct FrameContext
{
	SDLState &state;
	InputState &input;
	float deltaTime;
	GHandle rootObject;

	FrameContext(SDLState &state, InputState &input, float deltaTime) : state(state), input(input)
	{
		this->deltaTime = deltaTime;
		this->rootObject = rootObject;
	}
};