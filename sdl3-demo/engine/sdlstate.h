#pragma once

struct SDL_Window;

struct SDLState
{
	int logW, logH;
	const bool *keys;
	bool fullscreen;

	SDLState(const bool *keys) : keys(keys)
	{
		logW = 0;
		logH = 0;
		fullscreen = false;
	}
};

