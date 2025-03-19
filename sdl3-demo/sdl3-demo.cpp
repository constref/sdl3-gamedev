#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <format>

using namespace std;

struct SDLState
{
	int width;
	int height;
	int logW;
	int logH;
	SDL_Window *window;
	SDL_Renderer *renderer;
};

enum class PlayerState
{
	idle, running, shootRunning, shootStanding
};

class Animation
{
	int frameCount, frameNum;
	float time, length;

public:
	Animation(int frameCount, float length) : frameCount(frameCount), length(length)
	{
		frameNum = 0;
		time = 0;
	}

	void update(float deltaTime)
	{
		time += deltaTime;
		if (time >= length)
		{
			time = time - length;
		}
		frameNum = static_cast<int>(time / length * frameCount);
	}
	void reset() { time = 0; frameNum = 0; }
	void setTime(float time) { this->time = time; }
	float getTime() const { return time; }
	int getFrame() const { return frameNum; }
};

bool initialize(SDLState &state);
void cleanup(SDLState &state);

int main(int argc, char *argv[])
{
	SDLState state;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 320;

	if (!initialize(state))
	{
		return 1;
	}

	// load game assets and animations
	SDL_Texture *idleTex = IMG_LoadTexture(state.renderer, "data/idle.png");
	SDL_SetTextureScaleMode(idleTex, SDL_SCALEMODE_NEAREST);

	SDL_Texture *runTex = IMG_LoadTexture(state.renderer, "data/run.png");
	SDL_SetTextureScaleMode(runTex, SDL_SCALEMODE_NEAREST);

	SDL_Texture *shootRunTex = IMG_LoadTexture(state.renderer, "data/shoot_run.png");
	SDL_SetTextureScaleMode(shootRunTex, SDL_SCALEMODE_NEAREST);

	Animation animIdle(8, 1.6f);
	Animation animRun(4, 0.5f);
	Animation animShootRun(4, 0.5f);

	// setup game stuff
	uint64_t prevTime = SDL_GetTicks();
	const bool *keys = SDL_GetKeyboardState(nullptr);
	float playerPos = 0;
	double globalTime = 0;
	bool flipSprite = false;
	PlayerState playerState = PlayerState::idle;

	// start the game loop
	bool running = true;
	while (running)
	{
		// difference between start of last frame, and start of this frame in milliseconds
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds fraction

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
				case SDL_EVENT_WINDOW_RESIZED:
				{
					state.width = event.window.data1;
					state.height = event.window.data2;
					break;
				}
			}
		}

		// process game state
		Animation *anim = &animIdle;
		SDL_Texture *tex = idleTex;

		float moveAmount = 0;
		if (keys[SDL_SCANCODE_D])
		{
			flipSprite = false;
			moveAmount = 75.0f;
		}
		else if (keys[SDL_SCANCODE_A])
		{
			flipSprite = true;
			moveAmount = -75.0f;
		}

		if (moveAmount)
		{
			playerPos += moveAmount * deltaTime;
			if (keys[SDL_SCANCODE_J])
			{
				playerState = PlayerState::shootRunning;
				anim = &animShootRun;
				tex = shootRunTex;

			}
			else
			{
				playerState = PlayerState::running;
				anim = &animRun;
				tex = runTex;
			}

			// keep running and shoot-running animations in sync
			animRun.update(deltaTime);
			animShootRun.update(deltaTime);
		}
		else
		{
			// no longer moving, reset animations and switch to idle anim
			playerState = PlayerState::idle;
			animRun.reset();
			animShootRun.reset();
			animIdle.update(deltaTime);
		}

		// perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		SDL_FRect src{
			.x = anim->getFrame() * 32.0f,
			.y = 0,
			.w = 32,
			.h = 32
		};

		SDL_FRect dst{
			.x = playerPos,
			.y = state.logH - 32.0f,
			.w = 32,
			.h = 32
		};

		SDL_RenderTextureRotated(state.renderer, tex, &src, &dst, 0, nullptr,
			flipSprite ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
		globalTime += deltaTime;

		SDL_SetWindowTitle(state.window, std::format("{:.2f}  -  {:.2f} ({})",
			globalTime, animIdle.getTime(), animIdle.getFrame()).c_str());
	}

	SDL_DestroyTexture(idleTex);
	SDL_DestroyTexture(runTex);
	SDL_DestroyTexture(shootRunTex);
	cleanup(state);
	return 0;
}

bool initialize(SDLState &state)
{
	bool initSuccess = true;
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		initSuccess = false;
	}

	// create the window
	state.window = SDL_CreateWindow("SDL3 Demo", state.width, state.height, SDL_WINDOW_RESIZABLE);
	if (!state.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup(state);
		initSuccess = false;
	}

	// create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		cleanup(state);
		initSuccess = false;
	}

	// configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	return initSuccess;
}

void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}
