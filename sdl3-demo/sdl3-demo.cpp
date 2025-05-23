#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <vector>
#include <string>
#include <array>
#include <format>

#include "gameobject.h"

using namespace std;

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int width, height, logW, logH;
	const bool *keys;
	bool fullscreen;

	SDLState() : keys(SDL_GetKeyboardState(nullptr))
	{
		fullscreen = true;
	}
};

const size_t LAYER_IDX_BACKGROUND = 0;
const size_t LAYER_IDX_LEVEL = 1;
const size_t LAYER_IDX_CHARACTERS = 2;
const size_t LAYER_IDX_FOREGROUND = 3;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState
{
	std::array<std::vector<GameObject>, 4> layers;
	std::vector<GameObject> bullets;
	int playerIndex;
	SDL_FRect mapViewport;
	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	GameState(const SDLState &state)
	{
		playerIndex = -1;
		mapViewport = SDL_FRect{
			.x = 0, .y = 0,
			.w = static_cast<float>(state.logW),
			.h = static_cast<float>(state.logH)
		};
		bg2Scroll = bg3Scroll = bg4Scroll = 0;
		debugMode = false;
	}

	GameObject &player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; }
};

struct Resources
{
	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUN = 1;
	const int ANIM_PLAYER_SLIDE = 2;
	const int ANIM_PLAYER_SHOOT = 3;
	const int ANIM_PLAYER_SLIDE_SHOOT = 4;
	std::vector<Animation> playerAnims;
	const int ANIM_BULLET_MOVING = 0;
	const int ANIM_BULLET_HIT = 1;
	std::vector<Animation> bulletAnims;
	const int ANIM_ENEMY = 0;
	const int ANIM_ENEMY_HIT = 1;
	const int ANIM_ENEMY_DIE = 2;
	std::vector<Animation> enemyAnims;

	std::vector<SDL_Texture *> textures;
	SDL_Texture *texIdle, *texRun, *texBrick, *texGrass, *texGround, *texPanel,
		*texSlide, *texBg1, *texBg2, *texBg3, *texBg4, *texBullet, *texBulletHit,
		*texShoot, *texRunShoot, *texSlideShoot, *texEnemy, *texEnemyHit, *texEnemyDie;

	std::vector<Mix_Chunk *> chunks;
	Mix_Chunk *chunkShoot, *chunkShootHit, *chunkEnemyHit;
	Mix_Music *musicMain;

	SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filepath)
	{
		SDL_Texture *tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	Mix_Chunk *loadChunk(const std::string &filepath)
	{
		Mix_Chunk *chunk = Mix_LoadWAV(filepath.c_str());
		Mix_VolumeChunk(chunk, MIX_MAX_VOLUME / 2);
		chunks.push_back(chunk);
		return chunk;
	}

	void load(SDLState &state)
	{
		playerAnims.resize(5);
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
		bulletAnims.resize(2);
		bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.05f);
		bulletAnims[ANIM_BULLET_HIT] = Animation(4, 0.15f);
		enemyAnims.resize(3);
		enemyAnims[ANIM_ENEMY] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_HIT] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

		texIdle = loadTexture(state.renderer, "data/idle.png");
		texRun = loadTexture(state.renderer, "data/run.png");
		texSlide = loadTexture(state.renderer, "data/slide.png");
		texBrick = loadTexture(state.renderer, "data/tiles/brick.png");
		texGrass = loadTexture(state.renderer, "data/tiles/grass.png");
		texGround = loadTexture(state.renderer, "data/tiles/ground.png");
		texPanel = loadTexture(state.renderer, "data/tiles/panel.png");
		texBg1 = loadTexture(state.renderer, "data/bg/bg_layer1.png");
		texBg2 = loadTexture(state.renderer, "data/bg/bg_layer2.png");
		texBg3 = loadTexture(state.renderer, "data/bg/bg_layer3.png");
		texBg4 = loadTexture(state.renderer, "data/bg/bg_layer4.png");
		texBullet = loadTexture(state.renderer, "data/bullet.png");
		texBulletHit = loadTexture(state.renderer, "data/bullet_hit.png");
		texShoot = loadTexture(state.renderer, "data/shoot.png");
		texRunShoot = loadTexture(state.renderer, "data/shoot_run.png");
		texSlideShoot = loadTexture(state.renderer, "data/slide_shoot.png");
		texEnemy = loadTexture(state.renderer, "data/enemy.png");
		texEnemyHit = loadTexture(state.renderer, "data/enemy_hit.png");
		texEnemyDie = loadTexture(state.renderer, "data/enemy_die.png");

		chunkShoot = loadChunk("data/audio/shoot.wav");
		chunkShootHit = loadChunk("data/audio/wall_hit.wav");
		chunkEnemyHit = loadChunk("data/audio/shoot_hit.wav");

		musicMain = Mix_LoadMUS("data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");
	}

	void unload()
	{
		for (SDL_Texture *tex : textures)
		{
			SDL_DestroyTexture(tex);
		}

		for (Mix_Chunk *chunk : chunks)
		{
			Mix_FreeChunk(chunk);
		}

		Mix_FreeMusic(musicMain);
	}
};

bool initialize(SDLState &state);
void cleanup(SDLState &state);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj,
	float width, float height, float deltaTime);
void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime);
void createTiles(const SDLState &state, GameState &gs, const Resources &res);
void checkCollision(const SDLState &state, GameState &gs, Resources &res,
	GameObject &a, GameObject &b, float deltaTime);
void handleKeyInput(const SDLState &state, GameState &gs, GameObject &obj,
	SDL_Scancode key, bool keyDown);
void drawParalaxBackground(SDL_Renderer *renderer, SDL_Texture *texture,
	float xVelocity, float &scrollPos, float scrollFactor, float deltaTime);

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

	// load game assets
	Resources res;
	res.load(state);

	// setup game data
	GameState gs(state);
	createTiles(state, gs, res);
	uint64_t prevTime = SDL_GetTicks();

	Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
	Mix_PlayMusic(res.musicMain, -1);

	// start the game loop
	bool running = true;
	while (running)
	{
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f;

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
					handleKeyInput(state, gs, gs.player(), event.key.scancode, true);
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					handleKeyInput(state, gs, gs.player(), event.key.scancode, false);
					if (event.key.scancode == SDL_SCANCODE_F12)
					{
						gs.debugMode = !gs.debugMode;
					}
					else if (event.key.scancode == SDL_SCANCODE_F11)
					{
						state.fullscreen = !state.fullscreen;
						SDL_SetWindowFullscreen(state.window, state.fullscreen);
					}
					break;
				}
			}
		}

		// update all objects
		for (auto &layer : gs.layers)
		{
			for (GameObject &obj : layer)
			{
				update(state, gs, res, obj, deltaTime);
			}
		}

		// update bullets
		for (GameObject &bullet : gs.bullets)
		{
			update(state, gs, res, bullet, deltaTime);
		}

		// calculate viewport position
		gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;

		// perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		// draw background images
		SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
		drawParalaxBackground(state.renderer, res.texBg4, gs.player().velocity.x,
			gs.bg4Scroll, 0.075f, deltaTime);
		drawParalaxBackground(state.renderer, res.texBg3, gs.player().velocity.x,
			gs.bg3Scroll, 0.150f, deltaTime);
		drawParalaxBackground(state.renderer, res.texBg2, gs.player().velocity.x,
			gs.bg2Scroll, 0.3f, deltaTime);

		// draw all objects
		for (auto &layer : gs.layers)
		{
			for (GameObject &obj : layer)
			{
				drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
			}
		}

		// draw bullets
		for (GameObject &bullet : gs.bullets)
		{
			if (bullet.data.bullet.state != BulletState::inactive)
			{
				drawObject(state, gs, bullet, bullet.collider.w, bullet.collider.h, deltaTime);
			}
		}

		if (gs.debugMode)
		{
			// display some debug info
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5,
				std::format("S: {}, B: {}, G: {}",
					static_cast<int>(gs.player().data.player.state), gs.bullets.size(), gs.player().grounded).c_str());
		}

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
	}

	res.unload();
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
	SDL_SetRenderVSync(state.renderer, 1);

	// configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	// initialize the SDL_mixer library
	if (!Mix_OpenAudio(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating audio device", state.window);
		cleanup(state);
		initSuccess = false;
	}

	SDL_SetWindowFullscreen(state.window, state.fullscreen);

	return initSuccess;
}

void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}

void drawObject(const SDLState &state, GameState &gs, GameObject &obj,
	float width, float height, float deltaTime)
{
	float srcX = obj.currentAnimation != -1
		? obj.animations[obj.currentAnimation].currentFrame() * width
		: (obj.spriteFrame - 1) * width;
	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = width,
		.h = height
	};

	SDL_FRect dst{
		.x = obj.position.x - gs.mapViewport.x,
		.y = obj.position.y,
		.w = width,
		.h = height
	};

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	if (!obj.shouldFlash)
	{
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
	}
	else
	{
		// flash object with a redish tint
		SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.0f, 1.0f);
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
		SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);

		if (obj.flashTimer.step(deltaTime))
		{
			obj.shouldFlash = false;
		}
	}

	if (gs.debugMode)
	{
		SDL_FRect rectA{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y,
			.w = obj.collider.w,
			.h = obj.collider.h
		};
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);

		SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
		SDL_RenderFillRect(state.renderer, &rectA);
		SDL_FRect sensor{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y + obj.collider.h,
			.w = obj.collider.w, .h = 1
		};
		SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 150);
		SDL_RenderFillRect(state.renderer, &sensor);

		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
	}
}

void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime)
{
	// update the animation
	if (obj.currentAnimation != -1)
	{
		obj.animations[obj.currentAnimation].step(deltaTime);
	}

	if (obj.dynamic && !obj.grounded)
	{
		// apply some gravity
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	}

	float currentDirection = 0;
	if (obj.type == ObjectType::player)
	{
		if (state.keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
		}
		if (state.keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
		}
		Timer &weaponTimer = obj.data.player.weaponTimer;
		weaponTimer.step(deltaTime);

		const auto handleShooting = [&state, &gs, &res, &obj, &weaponTimer](
			SDL_Texture *tex, SDL_Texture *shootTex, int animIndex, int shootAnimIndex)
			{
				if (state.keys[SDL_SCANCODE_J])
				{
					// set shooting tex/anim
					obj.texture = shootTex;
					obj.currentAnimation = shootAnimIndex;

					if (weaponTimer.isTimeout())
					{
						weaponTimer.reset();
						// spawn some bullets
						GameObject bullet;
						bullet.data.bullet = BulletData();
						bullet.type = ObjectType::bullet;
						bullet.direction = gs.player().direction;
						bullet.texture = res.texBullet;
						bullet.currentAnimation = res.ANIM_BULLET_MOVING;
						bullet.collidable = true;
						bullet.collider = SDL_FRect{
							.x = 0, .y = 0,
							.w = static_cast<float>(res.texBullet->h),
							.h = static_cast<float>(res.texBullet->h)
						};
						const int yVariation = 40;
						const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
						bullet.velocity = glm::vec2(
							obj.velocity.x + 600.0f * obj.direction,
							yVelocity
						);
						bullet.maxSpeedX = 1000.0f;
						bullet.animations = res.bulletAnims;

						// adjust bullet start position
						const float left = 4;
						const float right = 24;
						const float t = (obj.direction + 1) / 2.0f; // results in a value of 0..1
						const float xOffset = left + right * t; // LERP between left and right based on direction
						bullet.position = glm::vec2(
							obj.position.x + xOffset,
							obj.position.y + TILE_SIZE / 2 + 1
						);

						// look for an inactive slot and overwrite the bullet
						bool foundInactive = false;
						for (int i = 0; i < gs.bullets.size() && !foundInactive; i++)
						{
							if (gs.bullets[i].data.bullet.state == BulletState::inactive)
							{
								foundInactive = true;
								gs.bullets[i] = bullet;
							}
						}
						// if no inactive slot was found, push a new bullet
						if (!foundInactive)
						{
							gs.bullets.push_back(bullet);
						}

						Mix_PlayChannel(-1, res.chunkShoot, 0);
					}
				}
				else
				{
					obj.texture = tex;
					obj.currentAnimation = animIndex;
				}
			};

		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				// switching to running state
				if (currentDirection)
				{
					obj.data.player.state = PlayerState::running;
				}
				else
				{
					// decelerate
					if (obj.velocity.x)
					{
						const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
						float amount = factor * obj.acceleration.x * deltaTime;
						if (std::abs(obj.velocity.x) < std::abs(amount))
						{
							obj.velocity.x = 0;
						}
						else
						{
							obj.velocity.x += amount;
						}
					}
				}
				handleShooting(res.texIdle, res.texShoot, res.ANIM_PLAYER_IDLE, res.ANIM_PLAYER_SHOOT);
				break;
			}
			case PlayerState::running:
			{
				// switching to idle state
				if (!currentDirection)
				{
					obj.data.player.state = PlayerState::idle;
				}

				// moving in opposite direction of velocity, sliding!
				if (obj.velocity.x * obj.direction < 0 && obj.grounded)
				{
					handleShooting(res.texSlide, res.texSlideShoot, res.ANIM_PLAYER_SLIDE, res.ANIM_PLAYER_SLIDE_SHOOT);
				}
				else
				{
					handleShooting(res.texRun, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
				}
				break;
			}
			case PlayerState::jumping:
			{
				handleShooting(res.texRun, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
				break;
			}
		}
	}
	else if (obj.type == ObjectType::bullet)
	{
		switch (obj.data.bullet.state)
		{
			case BulletState::moving:
			{
				if (obj.position.x - gs.mapViewport.x < 0 || // left edge
					obj.position.x - gs.mapViewport.x > state.logW || // right edge
					obj.position.y - gs.mapViewport.y < 0 || // top edge
					obj.position.y - gs.mapViewport.y > state.logH) // bottom edge
				{
					obj.data.bullet.state = BulletState::inactive;
				}
				break;
			}
			case BulletState::colliding:
			{
				if (obj.animations[obj.currentAnimation].isDone())
				{
					obj.data.bullet.state = BulletState::inactive;
				}
				break;
			}
		}
	}
	else if (obj.type == ObjectType::enemy)
	{
		EnemyData &d = obj.data.enemy;
		switch (d.state)
		{
			case EnemyState::shambling:
			{
				glm::vec2 playerDir = gs.player().position - obj.position;
				if (glm::length(playerDir) < 100)
				{
					currentDirection = playerDir.x < 0 ? -1 : 1;
					obj.acceleration = glm::vec2(30, 0);
				}
				else
				{
					obj.acceleration = glm::vec2(0);
					obj.velocity.x = 0;
				}
				break;
			}
			case EnemyState::damaged:
			{
				if (d.damageTimer.step(deltaTime))
				{
					d.state = EnemyState::shambling;
					obj.texture = res.texEnemy;
					obj.currentAnimation = res.ANIM_ENEMY;
				}
				break;
			}
			case EnemyState::dead:
			{
				obj.velocity.x = 0;
				if (obj.currentAnimation != -1 &&
					obj.animations[obj.currentAnimation].isDone())
				{
					// remove animation and set to last frame
					obj.currentAnimation = -1;
					obj.spriteFrame = 18;
				}
				break;
			}
		}
	}

	if (currentDirection)
	{
		obj.direction = currentDirection;
	}
	// add acceleration to velocity
	obj.velocity += currentDirection * obj.acceleration * deltaTime;
	if (std::abs(obj.velocity.x) > obj.maxSpeedX)
	{
		obj.velocity.x = currentDirection * obj.maxSpeedX;
	}

	// add velocity to position
	obj.position += obj.velocity * deltaTime;

	// handle collision detection
	if (obj.collidable)
	{
		bool foundGround = false;
		for (auto &layer : gs.layers)
		{
			for (GameObject &objB : layer)
			{
				if (&obj != &objB)
				{
					if (objB.collidable)
					{
						checkCollision(state, gs, res, obj, objB, deltaTime);

						if (objB.type == ObjectType::level)
						{
							// grounded sensor
							SDL_FRect sensor{
								.x = obj.position.x + obj.collider.x,
								.y = obj.position.y + obj.collider.y + obj.collider.h,
								.w = obj.collider.w, .h = 1
							};
							SDL_FRect rectB{
								.x = objB.position.x + objB.collider.x,
								.y = objB.position.y + objB.collider.y,
								.w = objB.collider.w, .h = objB.collider.h
							};
							SDL_FRect rectC{ 0 };
							if (SDL_GetRectIntersectionFloat(&sensor, &rectB, &rectC))
							{
								foundGround = true;
							}
						}
					}
				}
			}
		}
		if (obj.grounded != foundGround)
		{
			if (foundGround && obj.type == ObjectType::player)
			{
				obj.data.player.state = PlayerState::running;
			}
			obj.grounded = foundGround;
		}
	}
}

void collisionResponse(const SDLState &state, GameState &gs, Resources &res,
	const SDL_FRect &rectA, const SDL_FRect &rectB, const SDL_FRect &rectC,
	GameObject &objA, GameObject &objB, float deltaTime)
{
	const auto genericResponse = [&]()
		{
			if (rectC.w < rectC.h)
			{
				// horizontal collision
				if (objA.velocity.x > 0) // going right
				{
					objA.position.x -= rectC.w;
				}
				else if (objA.velocity.x < 0)
				{
					objA.position.x += rectC.w;
				}
				objA.velocity.x = 0;
			}
			else
			{
				// vertical collision
				if (objA.velocity.y > 0)
				{
					objA.position.y -= rectC.h; // going down
				}
				else if (objA.velocity.y < 0)
				{
					objA.position.y += rectC.h; // going up
				}
				objA.velocity.y = 0;
			}
		};

	// object we are checking
	if (objA.type == ObjectType::player)
	{
		// object it is colliding with
		switch (objB.type)
		{
			case ObjectType::level:
			{
				genericResponse();
				break;
			}
			case ObjectType::enemy:
			{
				if (objB.data.enemy.state != EnemyState::dead)
				{
					objA.velocity = glm::vec2(100, 0) * -objA.direction;
				}
				break;
			}
		}
	}
	else if (objA.type == ObjectType::bullet)
	{
		bool passthrough = false;
		switch (objA.data.bullet.state)
		{
			case BulletState::moving:
			{
				switch (objB.type)
				{
					case ObjectType::level:
					{
						Mix_PlayChannel(-1, res.chunkShootHit, 0);
						break;
					}
					case ObjectType::enemy:
					{
						EnemyData &d = objB.data.enemy;
						if (d.state != EnemyState::dead)
						{
							objB.direction = -objA.direction;
							objB.shouldFlash = true;
							objB.flashTimer.reset();
							objB.texture = res.texEnemyHit;
							objB.currentAnimation = res.ANIM_ENEMY_HIT;
							d.state = EnemyState::damaged;
							// damage the enemy and flag dead if needed
							d.healthPoints -= 10;
							if (d.healthPoints <= 0)
							{
								d.state = EnemyState::dead;
								objB.texture = res.texEnemyDie;
								objB.currentAnimation = res.ANIM_ENEMY_DIE;
							}
							Mix_PlayChannel(-1, res.chunkEnemyHit, 0);
						}
						else
						{
							// don't collide with dead enemies
							passthrough = true;
						}
						break;
					}
				}
				if (!passthrough)
				{
					genericResponse();
					objA.velocity *= 0;
					objA.data.bullet.state = BulletState::colliding;
					objA.texture = res.texBulletHit;
					objA.currentAnimation = res.ANIM_BULLET_HIT;
				}
				break;
			}
		}
	}
	else if (objA.type == ObjectType::enemy)
	{
		genericResponse();
	}
}

void checkCollision(const SDLState &state, GameState &gs, Resources &res,
	GameObject &a, GameObject &b, float deltaTime)
{
	SDL_FRect rectA{
		.x = a.position.x + a.collider.x,
		.y = a.position.y + a.collider.y,
		.w = a.collider.w,
		.h = a.collider.h
	};
	SDL_FRect rectB{
		.x = b.position.x + b.collider.x,
		.y = b.position.y + b.collider.y,
		.w = b.collider.w,
		.h = b.collider.h
	};
	SDL_FRect rectC{ 0 };

	if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC))
	{
		// found intersection, respond accordingly
		collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltaTime);
	}
}

void createTiles(const SDLState &state, GameState &gs, const Resources &res)
{
	/*
		1 - Ground
		2 - Panel
		3 - Enemy
		4 - Player
		5 - Grass
		6 - Brick
	*/
	short map[MAP_ROWS][MAP_COLS] = {
		4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 3, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 3, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 0, 2, 2, 2, 0, 0, 3, 2, 2, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 3, 0, 0, 3, 0, 2, 2, 2, 2, 2, 0, 0, 2, 2, 0, 3, 0, 0, 3, 0, 2, 3, 3, 3, 0, 2, 0, 3, 3, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 3,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	short background[MAP_ROWS][MAP_COLS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	short foreground[MAP_ROWS][MAP_COLS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	const auto loadMap = [&state, &gs, &res](short layer[MAP_ROWS][MAP_COLS])
		{
			const auto createObject = [&state](int r, int c, SDL_Texture *tex, ObjectType type)
				{
					GameObject o;
					o.type = type;
					o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
					o.texture = tex;
					o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE };
					return o;
				};

			for (int r = 0; r < MAP_ROWS; r++)
			{
				for (int c = 0; c < MAP_COLS; c++)
				{
					switch (layer[r][c])
					{
						case 1: // ground
						{
							GameObject o = createObject(r, c, res.texGround, ObjectType::level);
							o.collidable = true;
							gs.layers[LAYER_IDX_LEVEL].push_back(o);
							break;
						}
						case 2: // panel
						{
							GameObject o = createObject(r, c, res.texPanel, ObjectType::level);
							o.collidable = true;
							gs.layers[LAYER_IDX_LEVEL].push_back(o);
							break;
						}
						case 3: // enemy
						{
							GameObject o = createObject(r, c, res.texEnemy, ObjectType::enemy);
							o.data.enemy = EnemyData();
							o.currentAnimation = res.ANIM_ENEMY;
							o.animations = res.enemyAnims;
							o.collidable = true;
							o.collider = SDL_FRect{
								.x = 10, .y = 4, .w = 12, .h = 28
							};
							o.maxSpeedX = 15;
							o.dynamic = true;
							gs.layers[LAYER_IDX_CHARACTERS].push_back(o);
							break;
						}
						case 4: // player
						{
							GameObject player = createObject(r, c, res.texIdle, ObjectType::player);
							player.data.player = PlayerData();
							player.animations = res.playerAnims;
							player.currentAnimation = res.ANIM_PLAYER_IDLE;
							player.acceleration = glm::vec2(300, 0);
							player.maxSpeedX = 100;
							player.dynamic = true;
							player.collidable = true;
							player.collider = {
								.x = 11, .y = 6,
								.w = 10, .h = 26
							};
							gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
							gs.playerIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;
							break;
						}
						case 5: // grass
						{
							GameObject o = createObject(r, c, res.texGrass, ObjectType::level);
							gs.layers[LAYER_IDX_FOREGROUND].push_back(o);
							break;
						}
						case 6: // brick
						{
							GameObject o = createObject(r, c, res.texBrick, ObjectType::level);
							gs.layers[LAYER_IDX_BACKGROUND].push_back(o);
							break;
						}
					}
				}
			}
		};
	loadMap(map);
	loadMap(background);
	loadMap(foreground);
	assert(gs.playerIndex != -1);
}

void handleKeyInput(const SDLState &state, GameState &gs, GameObject &obj,
	SDL_Scancode key, bool keyDown)
{
	const float JUMP_FORCE = -200.0f;
	const auto jump = [&]()
		{
			if (key == SDL_SCANCODE_K && keyDown && obj.grounded)
			{
				obj.data.player.state = PlayerState::jumping;
				obj.velocity.y += JUMP_FORCE;
			}
		};

	if (obj.type == ObjectType::player)
	{
		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				jump();
				break;
			}
			case PlayerState::running:
			{
				jump();
				break;
			}
		}
	}
}

void drawParalaxBackground(SDL_Renderer *renderer, SDL_Texture *texture,
	float xVelocity, float &scrollPos, float scrollFactor, float deltaTime)
{
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w)
	{
		scrollPos = 0;
	}

	SDL_FRect dst{
		.x = scrollPos, .y = 30,
		.w = texture->w * 2.0f,
		.h = static_cast<float>(texture->h)
	};

	SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dst);
}
