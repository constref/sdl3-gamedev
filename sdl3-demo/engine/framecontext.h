#pragma once

#include <nodehandle.h>

struct SDLState;
struct Resources;
class InputState;

struct FrameContext
{
	SDLState &state;
	float deltaTime;
	double globalTime;
	long frameNumber;

	FrameContext(SDLState &state, float deltaTime, double globalTime, long frameNumber)
		: state(state)
	{
		this->deltaTime = deltaTime;
		this->globalTime = 0;
		this->frameNumber = frameNumber;
	}
};