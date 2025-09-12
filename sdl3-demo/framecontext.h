#pragma once

struct SDLState;
struct GameState;
struct Resources;
struct InputState;

struct FrameContext
{
	SDLState &state;
	GameState &gs;
	Resources &res;
	InputState &input;
	float deltaTime;

	FrameContext(SDLState &state, GameState &gs, Resources &res, InputState &input, float deltaTime)
		: state(state), gs(gs), res(res), input(input), deltaTime(deltaTime)
	{
	}
};