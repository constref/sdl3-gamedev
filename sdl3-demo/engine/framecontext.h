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
	long frameNumber;

	FrameContext(SDLState &state, InputState &input, float deltaTime, long frameNumber)
		: state(state), input(input)
	{
		this->deltaTime = deltaTime;
		this->rootObject = rootObject;
		this->frameNumber = frameNumber;
	}
};