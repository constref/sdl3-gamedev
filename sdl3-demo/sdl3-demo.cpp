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
	SDL_Rect collider;
	Animation *animation;
	SDL_Texture *texture;
	bool isGrounded;

	GameObject()
	{
		type = ObjectType::level;
		position = velocity = acceleration = glm::vec2(0, 0);
		collider = SDL_Rect{
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0
		};
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
	3 - Enemy
*/

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
short map[MAP_ROWS][MAP_COLS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 3, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 3, 2, 0, 3, 0, 2, 2, 2, 2, 2, 0, 0, 0, 3, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int TILE_SIZE = 32;
const int MAP_W = MAP_COLS * TILE_SIZE;
const int MAP_H = MAP_ROWS * TILE_SIZE;


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

	SDL_FRect mapViewport{
		.x = 0,
		.y = 0,
		.w = static_cast<float>(state.logW),
		.h = static_cast<float>(state.logH)
	};

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

	SDL_Texture *groundTex = IMG_LoadTexture(state.renderer, "data/tiles/ground.png");
	SDL_SetTextureScaleMode(groundTex, SDL_SCALEMODE_NEAREST);
	SDL_Texture *panelTex = IMG_LoadTexture(state.renderer, "data/tiles/panel.png");
	SDL_SetTextureScaleMode(panelTex, SDL_SCALEMODE_NEAREST);

	SDL_Texture *enemyTex = IMG_LoadTexture(state.renderer, "data/enemy.png");
	SDL_SetTextureScaleMode(enemyTex, SDL_SCALEMODE_NEAREST);
	Animation enemyAnim(4, 1.0f);

	// setup game data
	const int spriteSize = 32;
	const bool *keys = SDL_GetKeyboardState(nullptr);
	GameState gs;
	gs.player.position.x = mapViewport.w / 2 - spriteSize / 2;
	gs.player.collider = SDL_Rect{
		.x = 11, .y = 0, .w = 10, .h = spriteSize
	};

	float bg2Scroll = 0;
	float bg3Scroll = 0;
	float bg4Scroll = 0;

	// load map
	for (int r = 0; r < MAP_ROWS; ++r)
	{
		for (int c = 0; c < MAP_COLS; ++c)
		{
			const auto createObject = [&state](ObjectType type, int r, int c, SDL_Texture *tex)
			{
				GameObject o;
				o.type = type;
				o.texture = tex;
				o.position = glm::vec2(c * spriteSize, state.logH - (MAP_ROWS - r) * tex->h);
				o.velocity = glm::vec2(0, 0);
				o.acceleration = glm::vec2(0, 0);
				o.collider = SDL_Rect{
					.x = 0, .y = 0, .w = spriteSize, .h = spriteSize
				};

				return o;
			};

			switch (map[r][c])
			{
				case 1:
				{
					// ground
					GameObject o = createObject(ObjectType::level, r, c, groundTex);
					gs.objects.push_back(o);
					break;
				}
				case 2:
				{
					// paneling
					GameObject o = createObject(ObjectType::level, r, c, panelTex);
					gs.objects.push_back(o);
					break;
				}
				case 3:
				{
					// enemy
					GameObject o = createObject(ObjectType::enemy, r, c, enemyTex);
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
		float deltaTime = min((nowTime - gs.prevTime) / 1000.0f, 1 / 60.0f);

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

			// spawn some bullets
		};

		if (gs.playerState == PlayerState::idle)
		{
			// decelerate on idle
			if (gs.player.velocity.x)
			{
				// apply inverse force to decelerate
				const float fac = gs.player.velocity.x > 0 ? -1.0f : 1.0f;
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
			// and starting to move in the opposite direction
			if (gs.player.velocity.x * gs.direction < 0 && gs.player.isGrounded)
			{
				// sliding and shooting?
				handleShooting(&animSlideShoot, slideShootTex, &animSlide, slideTex);
			}
		}

		// apply some constant gravity if not grounded
		if (!gs.player.isGrounded)
		{
			gs.player.velocity += glm::vec2(0, 500) * deltaTime;
		}

		// check for collisions
		glm::vec2 newPos = gs.player.position;

		// x-axis movement and check
		newPos.x += gs.player.velocity.x * deltaTime;
		for (const GameObject &o : gs.objects)
		{
			SDL_Rect oRect{
				.x = static_cast<int>(o.position.x) + o.collider.x,
				.y = static_cast<int>(o.position.y) + o.collider.y,
				.w = o.collider.w, .h = o.collider.h
			};

			SDL_Rect pRect{
				.x = static_cast<int>(gs.player.velocity.x > 0 ? ceil(newPos.x) : newPos.x) + gs.player.collider.x,
				.y = static_cast<int>(newPos.y) + gs.player.collider.y,
				.w = gs.player.collider.w,
				.h = gs.player.collider.h
			};

			SDL_Rect result{ 0 };
			if (SDL_GetRectIntersection(&pRect, &oRect, &result))
			{
				if (gs.player.velocity.x > 0)
				{
					// going right
					newPos.x = static_cast<float>(oRect.x - oRect.w + gs.player.collider.x);
				}
				else if (gs.player.velocity.x < 0)
				{
					newPos.x = static_cast<float>(oRect.x + o.collider.w);
					newPos.x = static_cast<float>(oRect.x + oRect.w - gs.player.collider.x);
				}

				// bounce off enemies
				if (o.type == ObjectType::enemy)
				{
					gs.player.velocity.x *= -1.0f;
				}
				else
				{
					gs.player.velocity.x = 0;
				}
			}
		}

		// y-axis movement and check
		gs.player.isGrounded = false;
		newPos.y += gs.player.velocity.y * deltaTime;
		for (const GameObject &o : gs.objects)
		{
			SDL_Rect pRect{
				.x = static_cast<int>(newPos.x) + gs.player.collider.x,
				.y = static_cast<int>(newPos.y) + gs.player.collider.y,
				.w = gs.player.collider.w,
				.h = gs.player.collider.h
			};
			SDL_Rect oRect{
				.x = static_cast<int>(o.position.x),
				.y = static_cast<int>(o.position.y),
				.w = o.collider.w,
				.h = o.collider.h
			};

			SDL_Rect result{ 0 };
			if (SDL_GetRectIntersection(&pRect, &oRect, &result))
			{
				if (gs.player.velocity.y > 0)
				{
					// going down
					newPos.y = static_cast<float>(oRect.y - spriteSize);
					gs.player.isGrounded = true;
				}
				else if (gs.player.velocity.y < 0)
				{
					newPos.y = static_cast<float>(oRect.y + oRect.h);
				}

				if (o.type == ObjectType::enemy)
				{
					gs.player.velocity.y *= -1.0f;
				}
				else
				{
					gs.player.velocity.y = 0;
				}
			}
		}

		// use a sensor to check if on ground
		if (!gs.player.isGrounded)
		{
			SDL_Rect pRect{
				.x = static_cast<int>(newPos.x) + gs.player.collider.x,
				.y = static_cast<int>(newPos.y) + gs.player.collider.y,
				.w = gs.player.collider.w,
				.h = gs.player.collider.h
			};
			SDL_Rect groundSensor{
				.x = pRect.x,
				.y = pRect.y + pRect.h,
				.w = pRect.w,
				.h = 1
			};
			for (const GameObject &o : gs.objects)
			{
				if (o.type == ObjectType::level)
				{
					SDL_Rect oRect{
						.x = static_cast<int>(o.position.x),
						.y = static_cast<int>(o.position.y),
						.w = o.collider.w,
						.h = o.collider.h
					};
					if (SDL_HasRectIntersection(&groundSensor, &oRect))
					{
						gs.player.isGrounded = true;
						continue;
					}
				}
			}
		}

		// finally overwrite with the resolved position (if collisions occurred)
		const float moveXDiff = newPos.x - gs.player.position.x;
		gs.player.position = newPos;
		mapViewport.x += moveXDiff;

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
			if (o.type == ObjectType::level)
			{
				SDL_FRect src{
					.x = 0,
					.y = 0,
					.w = static_cast<float>(o.texture->w),
					.h = static_cast<float>(o.texture->h)
				};

				SDL_FRect dst{
					.x = o.position.x - mapViewport.x,
					.y = o.position.y,
					.w = static_cast<float>(o.texture->w),
					.h = static_cast<float>(o.texture->h)
				};
				SDL_RenderTexture(state.renderer, o.texture, &src, &dst);
			}
			else if (o.type == ObjectType::enemy)
			{
				SDL_FRect src{
					.x = static_cast<float>(enemyAnim.currentFrame() * spriteSize),
					.y = 0,
					.w = static_cast<float>(spriteSize),
					.h = static_cast<float>(spriteSize)
				};

				SDL_FRect dst{
					.x = o.position.x - mapViewport.x,
					.y = o.position.y,
					.w = static_cast<float>(spriteSize),
					.h = static_cast<float>(spriteSize)
				};

				glm::vec2 pDir = gs.player.position - o.position; // direction of player
				SDL_RenderTextureRotated(state.renderer, o.texture, &src, &dst, 0, nullptr,
					pDir.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
			}
		}
		enemyAnim.progress(deltaTime);

		// draw the player
		SDL_FRect src{
			.x = gs.player.animation->currentFrame() * static_cast<float>(spriteSize),
			.y = 0,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_FRect dst{
			.x = gs.player.position.x - mapViewport.x,
			.y = gs.player.position.y,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_RenderTextureRotated(state.renderer, gs.player.texture, &src, &dst, 0, nullptr,
			(gs.flipHorizontal) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

		// DEBUGGING
		//SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 255);
		//SDL_FRect colliderRect{
		//	.x = gs.player.position.x + static_cast<float>(gs.player.collider.x) - mapViewport.x,
		//	.y = gs.player.position.y + static_cast<float>(gs.player.collider.y) - mapViewport.y,
		//	.w = static_cast<float>(gs.player.collider.w),
		//	.h = static_cast<float>(gs.player.collider.h)
		//};
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
	SDL_DestroyTexture(enemyTex);
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
