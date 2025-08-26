#pragma once

struct SDLState;
struct GameState;
struct Resources;

struct FrameContext
{
	SDLState &state;
	GameState &gs;
	Resources &res;
	float deltaTime;

	FrameContext(SDLState &state, GameState &gs, Resources &res, float deltaTime)
		: state(state), gs(gs), res(res), deltaTime(deltaTime)
	{
	}
};