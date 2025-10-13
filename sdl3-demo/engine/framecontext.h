#pragma once

struct SDLState;
struct Resources;
class InputState;

struct FrameContext
{
	SDLState &state;
	InputState &input;
	float deltaTime;

	FrameContext(SDLState &state, InputState &input, float deltaTime)
		: state(state), input(input), deltaTime(deltaTime)
	{
	}
};