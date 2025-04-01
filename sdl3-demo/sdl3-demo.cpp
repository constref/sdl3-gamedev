#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <format>
#include <vector>
#include <glm/glm.hpp>

#include "animation.h"

using namespace std;

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int width, height, logW, logH;
};

enum class PlayerState
{
	idle, running
};

enum class ObjectType
{
	player, enemy, level
};

struct GameObject
{
	ObjectType type;
	glm::vec2 position, velocity, acceleration;
	Animation *animation;
	SDL_Texture *texture;
	bool isGrounded;

	GameObject()
	{
		type = ObjectType::level;
		position = velocity = acceleration = glm::vec2(0, 0);
		animation = nullptr;
		texture = nullptr;
		isGrounded = false;
	}
};

struct GameState
{
	PlayerState playerState;
	GameObject player;
	std::vector<GameObject> objects;
	float direction;
	bool flipHorizontal;
	bool isShooting;
	uint64_t prevTime;
	double globalTime;
	float maxSpeed;
	float jumpForce;

	GameState()
	{
		playerState = PlayerState::idle;
		direction = 0;
		flipHorizontal = false;
		isShooting = false;
		prevTime = SDL_GetTicks();
		globalTime = 0;
		maxSpeed = 100;
		jumpForce = -200;

		player.type = ObjectType::player;
		player.position = glm::vec2(0, 0);
		player.velocity = glm::vec2(0, 0);
		player.acceleration = glm::vec2(300, 0);
	}
};

/*
	1 - Ground
	2 - Panel
*/

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
short map[MAP_ROWS][MAP_COLS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

bool initialize(SDLState &state);
void handleInput(SDLState &state, GameState &gameState, SDL_Scancode key, bool isDown);
void drawBackgroundLayer(SDLState &state, GameState &gameState, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime);
void cleanup(SDLState &state);

int main(int argc, char *argv[])
{
	SDLState state;
	state.width = 1600;
	state.height = 900;
	state.logW = 512;
	state.logH = 288;

	if (!initialize(state))
	{
		return 1;
	}

	// load game assets
	SDL_Texture *idleTex = IMG_LoadTexture(state.renderer, "data/idle.png");
	SDL_SetTextureScaleMode(idleTex, SDL_SCALEMODE_NEAREST);
	Animation animIdle(8, 1.6f);

	SDL_Texture *runTex = IMG_LoadTexture(state.renderer, "data/run.png");
	SDL_SetTextureScaleMode(runTex, SDL_SCALEMODE_NEAREST);
	Animation animRun(4, 0.5f);

	SDL_Texture *shootRunTex = IMG_LoadTexture(state.renderer, "data/shoot_run.png");
	SDL_SetTextureScaleMode(shootRunTex, SDL_SCALEMODE_NEAREST);
	Animation animShootRun(4, 0.5f);

	SDL_Texture *shootTex = IMG_LoadTexture(state.renderer, "data/shoot.png");
	SDL_SetTextureScaleMode(shootTex, SDL_SCALEMODE_NEAREST);
	Animation animShoot(4, 0.5f);

	SDL_Texture *slideTex = IMG_LoadTexture(state.renderer, "data/slide.png");
	SDL_SetTextureScaleMode(slideTex, SDL_SCALEMODE_NEAREST);
	Animation animSlide(1, 1.0f);

	SDL_Texture *slideShootTex = IMG_LoadTexture(state.renderer, "data/slide_shoot.png");
	SDL_SetTextureScaleMode(slideShootTex, SDL_SCALEMODE_NEAREST);
	Animation animSlideShoot(4, 0.5f);

	SDL_Texture *bg1Tex = IMG_LoadTexture(state.renderer, "data/bg_layer1.png");
	SDL_SetTextureScaleMode(bg1Tex, SDL_SCALEMODE_NEAREST);
	SDL_Texture *bg2Tex = IMG_LoadTexture(state.renderer, "data/bg_layer2.png");
	SDL_SetTextureScaleMode(bg2Tex, SDL_SCALEMODE_NEAREST);
	SDL_Texture *bg3Tex = IMG_LoadTexture(state.renderer, "data/bg_layer3.png");
	SDL_SetTextureScaleMode(bg3Tex, SDL_SCALEMODE_NEAREST);
	SDL_Texture *bg4Tex = IMG_LoadTexture(state.renderer, "data/bg_layer4.png");
	SDL_SetTextureScaleMode(bg4Tex, SDL_SCALEMODE_NEAREST);

	SDL_Texture *groundTex = IMG_LoadTexture(state.renderer, "data/ground.png");
	SDL_SetTextureScaleMode(groundTex, SDL_SCALEMODE_NEAREST);
	SDL_Texture *panelTex = IMG_LoadTexture(state.renderer, "data/panel.png");
	SDL_SetTextureScaleMode(panelTex, SDL_SCALEMODE_NEAREST);

	// setup game data
	const bool *keys = SDL_GetKeyboardState(nullptr);
	GameState gs;

	const float floor = state.logH;
	const int spriteSize = 32;

	float bg2Scroll = 0;
	float bg3Scroll = 0;
	float bg4Scroll = 0;

	// load map
	for (int r = 0; r < MAP_ROWS; ++r)
	{
		for (int c = 0; c < MAP_COLS; ++c)
		{
			switch (map[r][c])
			{
				case 1:
				{
					// ground
					GameObject o;
					o.type = ObjectType::level;
					o.position = glm::vec2(c * spriteSize, state.logH - (MAP_ROWS - r) * groundTex->w);
					o.velocity = glm::vec2(0, 0);
					o.acceleration = glm::vec2(0, 0);
					o.texture = groundTex;
					gs.objects.push_back(o);
					break;
				}
				case 2:
				{
					// paneling
					GameObject o;
					o.type = ObjectType::level;
					o.position = glm::vec2(c * spriteSize, state.logH - (MAP_ROWS - r) * groundTex->w);
					o.velocity = glm::vec2(0, 0);
					o.acceleration = glm::vec2(0, 0);
					o.texture = panelTex;
					gs.objects.push_back(o);
					break;
				}
			}
		}
	}

	// start the game loop
	bool running = true;
	while (running)
	{
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - gs.prevTime) / 1000.0f;

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
				case SDL_EVENT_KEY_DOWN:
				{
					handleInput(state, gs, event.key.scancode, true);
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					handleInput(state, gs, event.key.scancode, false);
					break;
				}
			}
		}

		// repeated code used for handling shooting animations
		const auto handleShooting = [&](Animation *shootAnim, SDL_Texture *shootTex, Animation *nonShootAnim, SDL_Texture *nonShootTex)
		{
			if (gs.isShooting)
			{
				gs.player.animation = shootAnim;
				gs.player.texture = shootTex;
			}
			else
			{
				gs.player.animation = nonShootAnim;
				gs.player.texture = nonShootTex;
			}
		};

		if (gs.playerState == PlayerState::idle)
		{
			// decelerate on idle
			if (gs.player.velocity.x)
			{
				// apply inverse force to decelerate
				const float fac = gs.player.velocity.x > 0 ? -1 : 1;
				gs.player.velocity += fac * gs.player.acceleration * deltaTime;
				animRun.reset();
				animShootRun.reset();
			}

			// standing and shooting?
			handleShooting(&animShoot, shootTex, &animIdle, idleTex);
		}
		else if (gs.playerState == PlayerState::running)
		{
			gs.player.velocity += gs.direction * gs.player.acceleration * deltaTime;
			if (fabs(gs.player.velocity.x) > gs.maxSpeed)
			{
				gs.player.velocity.x = gs.direction * gs.maxSpeed;
			}

			// running and shooting?
			handleShooting(&animShootRun, shootRunTex, &animRun, runTex);

			// if velocity and direction have different signs, we're sliding
			// and moving in the opposite direction
			if (gs.player.velocity.x * gs.direction < 0)
			{
				// sliding and shooting?
				handleShooting(&animSlideShoot, slideShootTex, &animSlide, slideTex);
			}
		}

		if (!gs.player.isGrounded)
		{
			// apply some constant gravity
			gs.player.velocity += glm::vec2(0, 500) * deltaTime;
		}

		// check for collisions
		glm::vec2 newPos = gs.player.position;

		// x-axis movement
		newPos.x += gs.player.velocity.x * deltaTime;
		SDL_Rect pRect{
			.x = static_cast<int>(newPos.x),
			.y = static_cast<int>(newPos.y),
			.w = spriteSize,
			.h = spriteSize
		};

		for (const GameObject &o : gs.objects)
		{
			SDL_Rect oRect{
				.x = static_cast<int>(o.position.x),
				.y = static_cast<int>(o.position.y),
				.w = o.texture->w,
				.h = o.texture->h
			};

			pRect.x = static_cast<int>(newPos.x);
			pRect.y = static_cast<int>(newPos.y);

			SDL_Rect result{ 0 };
			if (SDL_GetRectIntersection(&pRect, &oRect, &result))
			{
				if (gs.player.velocity.x > 0)
				{
					// going right
					newPos.x = oRect.x - spriteSize;
					gs.player.velocity.x = 0;
				}
				else if (gs.player.velocity.x < 0)
				{
					newPos.x = oRect.x + oRect.w;
					gs.player.velocity.x = 0;
				}
			}
		}

		// y-axis movement
		gs.player.isGrounded = false;
		newPos.y += gs.player.velocity.y * deltaTime;
		for (const GameObject &o : gs.objects)
		{
			SDL_Rect oRect{
				.x = static_cast<int>(o.position.x),
				.y = static_cast<int>(o.position.y),
				.w = o.texture->w,
				.h = o.texture->h
			};
			pRect.x = static_cast<int>(newPos.x);
			pRect.y = static_cast<int>(newPos.y);

			SDL_Rect result{ 0 };
			if (SDL_GetRectIntersection(&pRect, &oRect, &result))
			{
				if (gs.player.velocity.y > 0)
				{
					// going down
					newPos.y = oRect.y - spriteSize;
					gs.player.velocity.y = 0;
					gs.player.isGrounded = true;
				}
				else if (gs.player.velocity.y < 0)
				{
					newPos.y = oRect.y + oRect.h;
					gs.player.velocity.y = 0;
				}
			}
		}

		// use a sensor to check if on ground
		if (!gs.player.isGrounded)
		{
			SDL_Rect groundSensor{
				.x = pRect.x,
				.y = pRect.y + pRect.h,
				.w = pRect.w,
				.h = 1
			};
			for (const GameObject &o : gs.objects)
			{
				SDL_Rect oRect{
					.x = static_cast<int>(o.position.x),
					.y = static_cast<int>(o.position.y),
					.w = o.texture->w,
					.h = o.texture->h
				};
				if (SDL_HasRectIntersection(&groundSensor, &oRect))
				{
					gs.player.isGrounded = true;
					continue;
				}
			}
		}

		// finally overwrite with the resolved position (if collisions occurred)
		gs.player.position = newPos;

		// move our selected animation forward
		gs.player.animation->progress(deltaTime);

		// perform drawing commands
		SDL_RenderTexture(state.renderer, bg1Tex, nullptr, nullptr);

		drawBackgroundLayer(state, gs, bg4Tex, bg4Scroll, 0.005f, deltaTime);
		drawBackgroundLayer(state, gs, bg3Tex, bg3Scroll, 0.025f, deltaTime);
		drawBackgroundLayer(state, gs, bg2Tex, bg2Scroll, 0.1f, deltaTime);

		// draw the level tiles
		for (const GameObject &o : gs.objects)
		{
			SDL_FRect src{
				.x = 0,
				.y = 0,
				.w = static_cast<float>(o.texture->w),
				.h = static_cast<float>(o.texture->h)
			};

			SDL_FRect dst{
				.x = o.position.x,
				.y = o.position.y,
				.w = static_cast<float>(o.texture->w),
				.h = static_cast<float>(o.texture->h)
			};
			SDL_RenderTexture(state.renderer, o.texture, &src, &dst);
		}

		SDL_FRect src{
			.x = gs.player.animation->currentFrame() * static_cast<float>(spriteSize),
			.y = 0,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_FRect dst{
			.x = gs.player.position.x,
			.y = gs.player.position.y,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_RenderTextureRotated(state.renderer, gs.player.texture, &src, &dst, 0, nullptr,
			(gs.flipHorizontal) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

		//// DEBUGGING
		//SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 255);
		//SDL_RenderRect(state.renderer, &colliderRect);

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		gs.prevTime = nowTime;

		// show some stats
		gs.globalTime += deltaTime;
		SDL_SetWindowTitle(state.window, std::format("Runtime: {:.3f} -- {} -- G({})",
			gs.globalTime, gs.direction, gs.player.isGrounded
		).c_str());
	}

	SDL_DestroyTexture(idleTex);
	SDL_DestroyTexture(runTex);
	SDL_DestroyTexture(shootRunTex);
	SDL_DestroyTexture(shootTex);
	SDL_DestroyTexture(slideTex);
	SDL_DestroyTexture(slideShootTex);
	SDL_DestroyTexture(bg1Tex);
	SDL_DestroyTexture(bg2Tex);
	SDL_DestroyTexture(bg3Tex);
	SDL_DestroyTexture(bg4Tex);
	SDL_DestroyTexture(groundTex);
	SDL_DestroyTexture(panelTex);
	cleanup(state);
	return 0;
}

void handleInput(SDLState &state, GameState &gs, SDL_Scancode key, bool isDown)
{
	switch (gs.playerState)
	{
		case PlayerState::idle:
		{
			if (key == SDL_SCANCODE_A && isDown)
			{
				gs.playerState = PlayerState::running;
				gs.direction = -1;
				gs.flipHorizontal = true;
			}
			else if (key == SDL_SCANCODE_D && isDown)
			{
				gs.playerState = PlayerState::running;
				gs.direction = 1;
				gs.flipHorizontal = false;
			}
			else if (key == SDL_SCANCODE_J)
			{
				gs.isShooting = isDown;
			}
			else if (key == SDL_SCANCODE_K && isDown)
			{
				// only jump if we aren't already jumping/falling
				if (gs.player.isGrounded)
				{
					gs.player.velocity.y = gs.jumpForce;
				}
			}
			break;
		}
		case PlayerState::running:
		{
			if (key == SDL_SCANCODE_A)
			{
				if (isDown)
				{
					gs.direction = -1;
					gs.flipHorizontal = true;
				}
				else if (gs.direction == -1)
				{
					// if we release the A key and are going left, go idle
					gs.playerState = PlayerState::idle;
					gs.direction = 0;
				}
			}
			else if (key == SDL_SCANCODE_D)
			{
				if (isDown)
				{
					gs.direction = 1;
					gs.flipHorizontal = false;
				}
				else if (gs.direction == 1) // going left
				{
					// if we release the D key and are going right, go idle
					gs.playerState = PlayerState::idle;
					gs.direction = 0;
				}
			}
			else if (key == SDL_SCANCODE_J)
			{
				gs.isShooting = isDown;
			}
			else if (key == SDL_SCANCODE_K && isDown)
			{
				if (gs.player.isGrounded)
				{
					gs.player.velocity.y = gs.jumpForce;
				}
			}
			break;
		}
	}
}

void drawBackgroundLayer(SDLState &state, GameState &gs, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime)
{
	SDL_FRect src{
		.x = 0,
		.y = 0,
		.w = static_cast<float>(tex->w),
		.h = static_cast<float>(tex->h)
	};

	SDL_FRect dst{
		.x = scrollPos,
		.y = 0,
		.w = static_cast<float>(state.logW * 2),
		.h = static_cast<float>(state.logH)
	};

	scrollPos -= gs.player.velocity.x * scrollFactor * deltaTime;
	if (scrollPos <= -tex->w)
	{
		scrollPos = 0;
	}
	SDL_RenderTextureTiled(state.renderer, tex, &src, 1, &dst);
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
	SDL_SetRenderVSync(state.renderer, 1);

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
