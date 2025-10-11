#pragma once

#include <SDL3/SDL.h>

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int width, height, logW, logH;
	const bool *keys;
	bool fullscreen;

	SDLState(int width, int height, int logW, int logH) : keys(SDL_GetKeyboardState(nullptr))
	{
		window = nullptr;
		renderer = nullptr;
		this->width = width;
		this->height = height;
		this->logW = logW;
		this->logH = logH;

		fullscreen = false;
	}
};

