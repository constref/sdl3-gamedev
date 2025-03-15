#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

using namespace std;

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
};

void cleanup(SDLState &state);

int main(int argc, char *argv[])
{
	SDLState state;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		return 1;
	}

	// create the window
	int width = 800;
	int height = 600;
	state.window = SDL_CreateWindow("SDL3 Demo", width, height, 0);
	if (!state.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup(state);
		return 1;
	}

	// create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		cleanup(state);
		return 1;
	}

	// start the game loop
	bool running = true;
	while (running)
	{
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
				{
					running = false;
					break;
				}
			}
		}

		// perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		SDL_RenderClear(state.renderer);

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
	}

	cleanup(state);
	return 0;
}

void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}
