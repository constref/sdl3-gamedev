#pragma once

#include <sdlstate.h>
#include <inputstate.h>
#include <gamestate.h>
#include <resources.h>

class Engine
{
	SDLState state;
	uint64_t prevTime = SDL_GetTicks();
	bool running = true;
	InputState inputState;

	GameState gs;

public:
	Engine();
	bool initialize();
	void run();
	void cleanup();
};