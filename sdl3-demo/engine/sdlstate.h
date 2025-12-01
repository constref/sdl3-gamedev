#pragma once

#include <SDL3/SDL.h>

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int width, height, logW, logH;
	const bool *keys;
	bool fullscreen;

	SDLState() : keys(SDL_GetKeyboardState(nullptr))
	{
		window = nullptr;
		renderer = nullptr;
		width = 0;
		height = 0;
		logW = 0;
		logH = 0;
		fullscreen = false;
	}

	static SDLState &global()
	{
		static SDLState state;
		return state;
	}

	bool initialize(int width, int height, int logW, int logH)
	{
		this->width = width;
		this->height = height;
		this->logW = logW;
		this->logH = logH;

		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
			return false;
		}

		// create the window
		window = SDL_CreateWindow("SDL3 Demo", width, height, SDL_WINDOW_RESIZABLE);
		if (!window)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
			cleanup();
			return false;
		}

		// create the renderer
		renderer = SDL_CreateRenderer(window, nullptr);
		if (!renderer)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", window);
			cleanup();
			return false;
		}
		//SDL_SetRenderVSync(renderer, 1);

		// configure presentation
		SDL_SetRenderLogicalPresentation(renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

		// initialize the SDL_mixer library
		//if (!MIX_Init())
		//{
		//	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating audio device", window);
		//	cleanup();
		//	initSuccess = false;
		//}

		SDL_SetWindowFullscreen(window, fullscreen);

		return true;
	}

	void cleanup()
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
};

