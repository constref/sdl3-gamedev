#pragma once

struct SDL_Window;

struct SDLState
{
	SDL_Window *window;
	int width, height, logW, logH;
	const bool *keys;
	bool fullscreen;

	SDLState(const bool *keys) : keys(keys)
	{
		window = nullptr;
		width = 0;
		height = 0;
		logW = 0;
		logH = 0;
		fullscreen = false;
	}
};

