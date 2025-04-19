#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <format>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <functional>
#include "gameobject.h"
#include "animation.h"

using namespace std;

const int LOGICAL_RES_WIDTH = 512;
const int LOGICAL_RES_HEIGHT = 288;

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int width, height;
	bool fullscreen;

	SDLState() : window(nullptr), renderer(nullptr)
	{
		width = 1600;
		height = 900;
		fullscreen = false;
	}
};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const float MAX_SPEED_PLAYER = 100;
const float MAX_SPEED_ENEMY = 45;
const float JUMP_FORCE = -200;

struct GameState
{
	std::array<std::vector<GameObject>, 2> objects;
	std::vector<GameObject> bullets;
	std::vector<GameObject> background;
	std::vector<GameObject> foreground;
	uint64_t prevTime;
	double globalTime;
	SDL_FRect mapViewport;
	size_t playerIndex;
	bool debugMode;
	bool paused;
	float bg2Scroll;
	float bg3Scroll;
	float bg4Scroll;

	GameState()
	{
		prevTime = SDL_GetTicks();
		globalTime = 0;
		mapViewport = SDL_FRect{
			.x = 0, .y = 0,
			.w = LOGICAL_RES_WIDTH, .h = LOGICAL_RES_HEIGHT,
		};
		playerIndex = -1;
		debugMode = false;
		paused = false;
		bg2Scroll = 0;
		bg3Scroll = 0;
		bg4Scroll = 0;
	}

	GameObject &player() { return objects[LAYER_IDX_CHARACTERS][playerIndex]; }
};

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;
const int MAP_W = MAP_COLS * TILE_SIZE;
const int MAP_H = MAP_ROWS * TILE_SIZE;

struct Resources
{
	std::vector<SDL_Texture *> textures;
	SDL_Texture *idleTex;
	SDL_Texture *runTex;
	SDL_Texture *shootRunTex;
	SDL_Texture *shootTex;
	SDL_Texture *slideTex;
	SDL_Texture *slideShootTex;
	SDL_Texture *bg1Tex;
	SDL_Texture *bg2Tex;
	SDL_Texture *bg3Tex;
	SDL_Texture *bg4Tex;
	SDL_Texture *groundTex;
	SDL_Texture *panelTex;
	SDL_Texture *enemyTex;
	SDL_Texture *enemyHitTex;
	SDL_Texture *enemyDieTex;
	SDL_Texture *bulletTex;
	SDL_Texture *bulletHitTex;
	SDL_Texture *grassTex;
	SDL_Texture *brickTex;

	Mix_Music *music;
	Mix_Chunk *chunkShoot;
	Mix_Chunk *chunkShootHit;
	Mix_Chunk *chunkWallHit;
	Mix_Chunk *chunkMonsterDie;

	int ANIM_IDX_PLAYER_IDLE = 0;
	int ANIM_IDX_PLAYER_RUN = 1;
	//int ANIM_IDX_PLAYER_SHOOT_RUN = 2;
	int ANIM_IDX_PLAYER_SHOOT = 2;
	int ANIM_IDX_PLAYER_SLIDE = 3;
	int ANIM_IDX_PLAYER_SLIDE_SHOOT = 4;
	std::vector<Animation> playerAnims;

	int ANIM_IDX_ENEMY_IDLE = 0;
	int ANIM_IDX_ENEMY_HIT = 1;
	int ANIM_IDX_ENEMY_DIE = 2;
	std::vector<Animation> enemyAnims;

	int ANIM_IDX_BULLET = 0;
	int ANIM_IDX_BULLET_HIT = 1;
	std::vector<Animation> bulletAnims;

	SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filePath)
	{
		SDL_Texture *tex = IMG_LoadTexture(renderer, filePath.c_str());
		textures.push_back(tex);
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		return tex;
	}

	void load(SDLState &state)
	{
		// load all textures
		idleTex = loadTexture(state.renderer, "data/idle.png");
		runTex = loadTexture(state.renderer, "data/run.png");
		shootRunTex = loadTexture(state.renderer, "data/shoot_run.png");
		shootTex = loadTexture(state.renderer, "data/shoot.png");
		slideTex = loadTexture(state.renderer, "data/slide.png");
		slideShootTex = loadTexture(state.renderer, "data/slide_shoot.png");
		bg1Tex = loadTexture(state.renderer, "data/bg_layer1.png");
		bg2Tex = loadTexture(state.renderer, "data/bg_layer2.png");
		bg3Tex = loadTexture(state.renderer, "data/bg_layer3.png");
		bg4Tex = loadTexture(state.renderer, "data/bg_layer4.png");
		groundTex = loadTexture(state.renderer, "data/tiles/ground.png");
		panelTex = loadTexture(state.renderer, "data/tiles/panel.png");
		enemyTex = loadTexture(state.renderer, "data/enemy.png");
		enemyHitTex = loadTexture(state.renderer, "data/enemy_hit.png");
		enemyDieTex = loadTexture(state.renderer, "data/enemy_die.png");
		bulletTex = loadTexture(state.renderer, "data/bullet.png");
		bulletHitTex = loadTexture(state.renderer, "data/bullet_hit.png");
		grassTex = loadTexture(state.renderer, "data/tiles/grass.png");
		brickTex = loadTexture(state.renderer, "data/tiles/brick.png");

		// load audio files
		music = Mix_LoadMUS("data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");
		chunkShoot = Mix_LoadWAV("data/audio/shoot.wav");
		Mix_VolumeChunk(chunkShoot, MIX_MAX_VOLUME / 2);
		chunkShootHit = Mix_LoadWAV("data/audio/shoot_hit.wav");
		Mix_VolumeChunk(chunkShootHit, MIX_MAX_VOLUME / 2);
		chunkWallHit = Mix_LoadWAV("data/audio/wall_hit.wav");
		Mix_VolumeChunk(chunkWallHit, MIX_MAX_VOLUME / 2);
		chunkMonsterDie = Mix_LoadWAV("data/audio/monster_die.wav");
		Mix_VolumeChunk(chunkMonsterDie, MIX_MAX_VOLUME / 2);

		// setup animations
		playerAnims.resize(6);
		playerAnims[ANIM_IDX_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_IDX_PLAYER_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_IDX_PLAYER_SHOOT] = Animation(4, 0.5f);
		//playerAnims[ANIM_IDX_PLAYER_SHOOT_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_IDX_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnims[ANIM_IDX_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
		enemyAnims.resize(3);
		enemyAnims[ANIM_IDX_ENEMY_IDLE] = Animation(8, 1.0f);
		enemyAnims[ANIM_IDX_ENEMY_HIT] = Animation(8, 1.0f);
		enemyAnims[ANIM_IDX_ENEMY_DIE] = Animation(18, 2.0f, true);
		bulletAnims.resize(2);
		bulletAnims[ANIM_IDX_BULLET] = Animation(4, 0.05f);
		bulletAnims[ANIM_IDX_BULLET_HIT] = Animation(4, 0.15f, true);
	}

	void unload()
	{
		// clean up all textures
		for (SDL_Texture *texture : textures)
		{
			SDL_DestroyTexture(texture);
		}

		Mix_FreeMusic(music);
		Mix_FreeChunk(chunkShoot);
		Mix_FreeChunk(chunkShootHit);
		Mix_FreeChunk(chunkWallHit);
		Mix_FreeChunk(chunkMonsterDie);
	}
};

bool initialize(SDLState &state);
void createTiles(const SDLState &state, const Resources &res, GameState &gs);
void handleInput(const SDLState &state, GameState &gs, GameObject &obj, SDL_Scancode key, bool isKeyDown);
void update(const SDLState &state, GameState &gs, GameObject &obj, Resources &res, const bool *keys, float deltaTime);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime);
void drawParalaxLayer(SDL_Renderer *renderer, float xVel, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime);
void checkCollision(const SDLState &state, GameState &gs, const Resources &res, GameObject &a, GameObject &b, float deltaTime);
void cleanup(SDLState &state);

int main(int argc, char *argv[])
{
	SDLState state;
	if (!initialize(state))
	{
		return 1;
	}

	// load all game resources
	Resources res;
	res.load(state);

	// setup game data
	const bool *keys = SDL_GetKeyboardState(nullptr);
	GameState gs;

	// generate tiles from map layer arrays
	createTiles(state, res, gs);

	Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
	Mix_PlayMusic(res.music, -1);

	// start the game loop
	bool running = true;
	while (running)
	{
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = min((nowTime - gs.prevTime) / 1000.0f, 1 / 60.0f);
		if (gs.paused)
		{
			deltaTime *= 0;
		}

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
					if (!gs.paused)
					{
						handleInput(state, gs, gs.player(), event.key.scancode, true);
					}
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					switch (event.key.scancode)
					{
						case SDL_SCANCODE_F11: // toggle fullscreen
						{
							state.fullscreen = !state.fullscreen;
							SDL_SetWindowFullscreen(state.window, state.fullscreen);
							break;
						}
						case SDL_SCANCODE_F12: // toggle debug-mode
						{
							gs.debugMode = !gs.debugMode;
							break;
						}
						case SDL_SCANCODE_P: // pause
						{
							gs.paused = !gs.paused;
							break;
						}
						case SDL_SCANCODE_F8: // restart level
						{
							gs = GameState();
							createTiles(state, res, gs);
							break;
						}
					}
					if (!gs.paused)
					{
						handleInput(state, gs, gs.player(), event.key.scancode, false);
					}
					break;
				}
			}
		}

		for (auto &objLayer : gs.objects)
		{
			for (GameObject &a : objLayer)
			{
				// apply some constant gravity if not grounded
				if (a.dynamic && !a.isGrounded)
				{
					a.velocity += glm::vec2(0, 500) * deltaTime;
				}
				update(state, gs, a, res, keys, deltaTime);

				// apply velocity after all updates are complete
				// we never update position directly in update()
				a.position += a.velocity * deltaTime;

				// collision detection and response
				for (auto &objLayer : gs.objects)
				{
					for (GameObject &b : objLayer)
					{
						if (&a != &b)
						{
							checkCollision(state, gs, res, a, b, deltaTime);
						}
					}
				}

				// update animation time
				if (a.animations.size())
				{
					a.animations[a.currentAnimation].progress(deltaTime);
				}

				// use a sensor to check if on ground
				SDL_FRect sensorRect{
					.x = a.position.x + a.collider.x,
					.y = a.position.y + a.collider.y + a.collider.h,
					.w = a.collider.w, .h = 1
				};
				bool foundGround = false;
				for (auto &objLayer : gs.objects)
				{
					for (GameObject &o : objLayer)
					{
						if (o.type == ObjectType::level)
						{
							SDL_FRect oRect{
								.x = o.position.x, .y = o.position.y,
								.w = o.collider.w, .h = o.collider.h
							};
							SDL_FRect cRect{ 0 };
							if (SDL_GetRectIntersectionFloat(&sensorRect, &oRect, &cRect))
							{
								foundGround = true;
							}
						}
					}
				}
				if (!a.isGrounded && foundGround)
				{
					if (a.type == ObjectType::player)
					{
						// trigger landing event
						a.data.player.state = a.direction ? PlayerState::running : PlayerState::idle;
					}
				}
				a.isGrounded = foundGround;
			}
		}

		// update bullets
		for (GameObject &bullet : gs.bullets)
		{
			if (bullet.data.bullet.state != BulletState::inactive)
			{
				bullet.position += bullet.velocity * deltaTime;
				bullet.animations[bullet.currentAnimation].progress(deltaTime);
				update(state, gs, bullet, res, keys, deltaTime);

				for (auto &objLayer : gs.objects)
				{
					for (GameObject &obj : objLayer)
					{
						checkCollision(state, gs, res, bullet, obj, deltaTime);
					}
				}
			}
		}

		// calculate viewport movement
		gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;

		// draw background/paralax images
		SDL_RenderTexture(state.renderer, res.bg1Tex, nullptr, nullptr); // furthest first
		drawParalaxLayer(state.renderer, gs.player().velocity.x, res.bg4Tex, gs.bg4Scroll, 0.075f, deltaTime);
		drawParalaxLayer(state.renderer, gs.player().velocity.x, res.bg3Tex, gs.bg3Scroll, 0.150f, deltaTime);
		drawParalaxLayer(state.renderer, gs.player().velocity.x, res.bg2Tex, gs.bg2Scroll, 0.3f, deltaTime);

		// draw background sprites
		for (GameObject &obj : gs.background)
		{
			drawObject(state, gs, obj, deltaTime);
		}

		// draw the game objects
		for (auto &objLayer : gs.objects)
		{
			for (GameObject &obj : objLayer)
			{
				drawObject(state, gs, obj, deltaTime);
			}
		}

		// draw foreground sprites
		for (GameObject &obj : gs.foreground)
		{
			drawObject(state, gs, obj, deltaTime);
		}

		// draw bullets
		for (GameObject &b : gs.bullets)
		{
			if (b.data.bullet.state != BulletState::inactive)
			{
				SDL_FRect src{
					.x = static_cast<float>(b.animations[b.currentAnimation].currentFrame() * b.texture->h),
					.y = 0,
					.w = static_cast<float>(b.texture->h),
					.h = static_cast<float>(b.texture->h)
				};

				SDL_FRect dst{
					.x = b.position.x - gs.mapViewport.x,
					.y = b.position.y - gs.mapViewport.y,
					.w = static_cast<float>(b.texture->h),
					.h = static_cast<float>(b.texture->h)
				};
				SDL_RenderTextureRotated(state.renderer, b.texture, &src, &dst, 0, nullptr, b.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
			}
		}

		if (gs.debugMode)
		{
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5,
				std::format("Runtime: {:.1f}  {}  {}  G({}) B({}, ({}, {})",
					gs.globalTime, static_cast<int>(gs.player().data.player.state),
					gs.player().direction, gs.player().isGrounded, gs.bullets.size(),
					gs.player().velocity.x, gs.player().velocity.y).c_str());
		}

		if (gs.paused)
		{
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, LOGICAL_RES_WIDTH / 2.0f - 25.0f, LOGICAL_RES_HEIGHT / 2.0f, "PAUSED");
		}

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		gs.prevTime = nowTime;

		// show some stats
		gs.globalTime += deltaTime;
	}

	res.unload();
	cleanup(state);
	return 0;
}

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime)
{
	int currentFrame = obj.animations.size() ? obj.animations[obj.currentAnimation].currentFrame() : 0;
	SDL_FRect src{
		.x = static_cast<float>(currentFrame * TILE_SIZE),
		.y = 0,
		.w = static_cast<float>(TILE_SIZE),
		.h = static_cast<float>(TILE_SIZE)
	};

	SDL_FRect dst{
		.x = obj.position.x - gs.mapViewport.x,
		.y = obj.position.y,
		.w = static_cast<float>(TILE_SIZE),
		.h = static_cast<float>(TILE_SIZE)
	};

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	if (!obj.shouldFlash)
	{
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
	}
	else
	{
		SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.0f, 1.0f);
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
		SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);

		if (obj.flashTimer.step(deltaTime))
		{
			obj.shouldFlash = false;
		}
	}

	if (gs.debugMode &&
		obj.type != ObjectType::background &&
		obj.type != ObjectType::foreground)
	{
		SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 100);
		SDL_FRect colliderRect{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y - gs.mapViewport.y,
			.w = obj.collider.w, .h = obj.collider.h
		};
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(state.renderer, &colliderRect);
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
	}
}

void handleInput(const SDLState &state, GameState &gs, GameObject &obj, SDL_Scancode key, bool isKeyDown)
{
	const auto performJump = [&gs, &obj]()
	{
		if (obj.isGrounded)
		{
			obj.data.player.state = PlayerState::jumping;
			obj.velocity.y = JUMP_FORCE;
		}
	};

	if (obj.type == ObjectType::player)
	{
		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				if (key == SDL_SCANCODE_K && isKeyDown)
				{
					performJump();
				}
				else if (key == SDL_SCANCODE_1 && isKeyDown)
				{
					obj.data.player.weaponTimer = Timer(PISTOL_TIME);
					break;
				}
				else if (key == SDL_SCANCODE_2 && isKeyDown)
				{
					obj.data.player.weaponTimer = Timer(ASSAULT_RIFLE_TIME);
					break;
				}
				break;
			}
			case PlayerState::running:
			{
				if (key == SDL_SCANCODE_K && isKeyDown)
				{
					performJump();
				}
				break;
			}
			case PlayerState::jumping:
			{
				break;
			}
		}
	}
}

void update(const SDLState &state, GameState &gs, GameObject &obj, Resources &res, const bool *keys, float deltaTime)
{
	// function to accelerate and limit speed
	const auto applyAcceleration = [deltaTime](GameObject &obj, float direction, float maxSpeed)
	{
		obj.velocity += direction * obj.acceleration * deltaTime;
		if (std::abs(obj.velocity.x) > maxSpeed)
		{
			obj.velocity.x = direction * maxSpeed;
		}
	};

	if (obj.type == ObjectType::player)
	{
		// always add time to weapon timer
		obj.data.player.weaponTimer.step(deltaTime);

		// repeated code used for handling shooting animations
		const auto handleShooting = [&](const int shootAnim, SDL_Texture *shootTex, const int nonShootAnim, SDL_Texture *nonShootTex)
		{
			if (obj.data.player.isShooting)
			{
				obj.currentAnimation = shootAnim;
				obj.texture = shootTex;

				if (obj.data.player.weaponTimer.isTimedOut())
				{
					obj.data.player.weaponTimer.reset();

					// spawn some bullets
					const float left = 4;
					const float right = 24;
					const float t = (obj.direction + 1) / 2.0f; // -1/1 + 1 = 0/2 divided by 2.0f -> 0.0f/1.0f
					const float xOffset = left + right * t; // lerp

					GameObject b;
					b.type = ObjectType::bullet;
					b.data.bullet = BulletData();
					b.position = obj.position + glm::vec2(xOffset, TILE_SIZE / 2 + 1);
					int yVelRand = SDL_rand(40) - 20; // -20 to 20
					b.velocity = glm::vec2(obj.velocity.x + 600.0f, yVelRand) * obj.direction;
					b.texture = res.bulletTex;
					b.animations = res.bulletAnims;
					b.currentAnimation = res.ANIM_IDX_BULLET;
					b.collider = SDL_FRect{
						.x = 0, .y = 0,
						.w = static_cast<float>(res.bulletTex->h),
						.h = static_cast<float>(res.bulletTex->h)
					};
					b.direction = obj.direction; // bullet direction is the player's direction

					// reuse an inactive bullet or push a new one
					bool foundInactive = false;
					size_t i = 0;
					while (!foundInactive && i < gs.bullets.size())
					{
						if (gs.bullets[i].data.bullet.state == BulletState::inactive)
						{
							gs.bullets[i] = b;
							foundInactive = true;
						}
						i++;
					}
					if (!foundInactive)
					{
						gs.bullets.push_back(b);
					}

					Mix_PlayChannel(-1, res.chunkShoot, 0);
				}
			}
			else
			{
				obj.currentAnimation = nonShootAnim;
				obj.texture = nonShootTex;
			}
		};

		// continuous input polling, running and/or shooting
		float currentDirection = 0;
		if (keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
		}
		if (keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
		}
		if (currentDirection)
		{
			obj.direction = currentDirection;
		}
		applyAcceleration(obj, currentDirection, MAX_SPEED_PLAYER);
		obj.data.player.isShooting = keys[SDL_SCANCODE_J];

		// handle state specifics
		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				// if holding a direction, switch to the running state
				if (currentDirection)
				{
					obj.data.player.state = PlayerState::running;
				}
				else
				{
					// decelerate horizontally
					if (obj.velocity.x)
					{
						// apply inverse force to decelerate
						const float fac = obj.velocity.x > 0 ? -1.5f : 1.5f;
						float decAmount = fac * obj.acceleration.x * deltaTime;
						// if the velocity left is less than the per-frame deceleration amount, just set to zero
						if (std::abs(obj.velocity.x) < decAmount)
						{
							obj.velocity.x = 0;
						}
						else
						{
							obj.velocity.x += decAmount;
						}
					}
					// standing and shooting?
					handleShooting(res.ANIM_IDX_PLAYER_SHOOT, res.shootTex, res.ANIM_IDX_PLAYER_IDLE, res.idleTex);
				}
				break;
			}
			case PlayerState::running:
			{
				// if direction is 0, then we're idling
				if (!currentDirection)
				{
					obj.data.player.state = PlayerState::idle;
				}
				else
				{
					// running and shooting?
					handleShooting(res.ANIM_IDX_PLAYER_RUN, res.shootRunTex, res.ANIM_IDX_PLAYER_RUN, res.runTex);

					// if velocity and direction have different signs, we're sliding
					// and starting to move in the opposite direction
					if (obj.velocity.x * obj.direction < 0 && obj.isGrounded)
					{
						// sliding and shooting?
						handleShooting(res.ANIM_IDX_PLAYER_SLIDE_SHOOT, res.slideShootTex, res.ANIM_IDX_PLAYER_SLIDE, res.slideTex);
					}
				}
				break;
			}
			case PlayerState::jumping:
			{
				handleShooting(res.ANIM_IDX_PLAYER_RUN, res.shootRunTex, res.ANIM_IDX_PLAYER_RUN, res.runTex);
				break;
			}
		}
	}
	else if (obj.type == ObjectType::bullet)
	{
		// deactivate bullets that are off-screen
		if (obj.position.x - gs.mapViewport.x < 0 || obj.position.x - gs.mapViewport.x > LOGICAL_RES_WIDTH ||
			obj.position.y - gs.mapViewport.y < 0 || obj.position.y - gs.mapViewport.y > LOGICAL_RES_HEIGHT)
		{
			obj.data.bullet.state = BulletState::inactive;
		}

		switch (obj.data.bullet.state)
		{
			case BulletState::flying:
			{
				if (obj.velocity.x == 0)
				{
					// switching to a colliding state and animation
					obj.data.bullet.state = BulletState::disintagrating;

					obj.texture = res.bulletHitTex;
					obj.currentAnimation = res.ANIM_IDX_BULLET_HIT;
				}
				break;
			}
			case BulletState::disintagrating:
			{
				if (obj.animations[obj.currentAnimation].getLoopCount() > 0)
				{
					// animation has played, remove bullet
					obj.data.bullet.state = BulletState::inactive;
				}
				break;
			}
		}
	}
	else if (obj.type == ObjectType::enemy)
	{
		glm::vec2 dirToPlayer = gs.player().position - obj.position;
		const float dist = glm::length(dirToPlayer);
		glm::vec2 dirNorm = glm::normalize(dirToPlayer);

		switch (obj.data.enemy.state)
		{
			case EnemyState::shambling:
			{
				if (dist < 200)
				{
					if (obj.data.enemy.thinkTimer.step(deltaTime))
					{
						obj.direction = dirNorm.x < 0 ? -1.0f : 1.0f;
					}
					applyAcceleration(obj, obj.direction, MAX_SPEED_ENEMY);
				}
				break;
			}
			case EnemyState::damaged:
			{
				// check if enemy is dead
				if (obj.data.enemy.hp <= 0)
				{
					obj.data.enemy.state = EnemyState::dead;
					obj.acceleration.x = 0;
					obj.velocity.x = 0;
					obj.texture = res.enemyDieTex;
					obj.currentAnimation = res.ANIM_IDX_ENEMY_DIE;

					Mix_PlayChannel(-1, res.chunkMonsterDie, 0);
				}
				else
				{
					if (obj.data.enemy.dmgTimer.step(deltaTime))
					{
						// once the damaged timer runs out, switch back to shambling
						obj.data.enemy.state = EnemyState::shambling;
						obj.texture = res.enemyTex;
						obj.currentAnimation = res.ANIM_IDX_ENEMY_IDLE;
					}
				}
				break;
			}
		}
	}
}

void drawParalaxLayer(SDL_Renderer *renderer, float xVel, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime)
{
	SDL_FRect src{
		.x = 0, .y = 0,
		.w = static_cast<float>(tex->w),
		.h = static_cast<float>(tex->h)
	};
	SDL_FRect dst{
		.x = scrollPos, .y = 10,
		.w = static_cast<float>(tex->w * 2), // repeat texture 2x horizontally
		.h = static_cast<float>(tex->h)
	};

	scrollPos -= xVel * scrollFactor * deltaTime;
	if (scrollPos <= -tex->w)
	{
		scrollPos = 0;
	}
	SDL_RenderTextureTiled(renderer, tex, &src, 1, &dst);
}

void createTiles(const SDLState &state, const Resources &res, GameState &gs)
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

	short foreground[MAP_ROWS][MAP_COLS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	short background[MAP_ROWS][MAP_COLS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	const auto loadLayer = [&state, &res, &gs](short layer[MAP_ROWS][MAP_COLS])
	{
		for (int r = 0; r < MAP_ROWS; ++r)
		{
			for (int c = 0; c < MAP_COLS; ++c)
			{
				const auto createObject = [&state](ObjectType type, int r, int c, SDL_Texture *tex)
				{
					GameObject o;
					o.data.level = LevelData();
					o.type = type;
					o.texture = tex;
					o.position = glm::vec2(c * TILE_SIZE, LOGICAL_RES_HEIGHT - (MAP_ROWS - r) * tex->h);
					o.velocity = glm::vec2(0, 0);
					o.acceleration = glm::vec2(0, 0);
					o.collider = SDL_FRect{
						.x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE
					};
					return o;
				};

				switch (layer[r][c])
				{
					case 1: // ground
					{
						GameObject o = createObject(ObjectType::level, r, c, res.groundTex);
						gs.objects[LAYER_IDX_LEVEL].push_back(o);
						break;
					}
					case 2: // paneling
					{
						GameObject o = createObject(ObjectType::level, r, c, res.panelTex);
						gs.objects[LAYER_IDX_LEVEL].push_back(o);
						break;
					}
					case 3: // enemy
					{
						GameObject o = createObject(ObjectType::enemy, r, c, res.enemyTex);
						o.data.enemy = EnemyData();
						o.dynamic = true;
						o.acceleration.x = 90;
						o.collider = SDL_FRect{
							.x = 10, .y = 4, .w = 12, .h = TILE_SIZE - 4
						};
						o.animations = res.enemyAnims;
						gs.objects[LAYER_IDX_CHARACTERS].push_back(o);
						break;
					}
					case 4: // player
					{
						GameObject o = createObject(ObjectType::player, r, c, res.idleTex);
						o.type = ObjectType::player;
						o.data.player = PlayerData();
						o.dynamic = true;
						o.velocity = glm::vec2(0, 0);
						o.acceleration = glm::vec2(300, 0);
						o.collider = SDL_FRect{
							.x = 11, .y = 6, .w = 10, .h = 26
						};

						o.animations = res.playerAnims;
						gs.objects[LAYER_IDX_CHARACTERS].push_back(o);
						gs.playerIndex = gs.objects[LAYER_IDX_CHARACTERS].size() - 1;
						break;
					}
					case 5: // grass
					{
						GameObject o = createObject(ObjectType::foreground, r, c, res.grassTex);
						gs.foreground.push_back(o);
						break;
					}
					case 6: // brick
					{
						GameObject o = createObject(ObjectType::background, r, c, res.brickTex);
						gs.background.push_back(o);
						break;
					}
				}
			}
		}
	};
	loadLayer(map);
	loadLayer(foreground);
	loadLayer(background);
}

void collisionResponse(const SDLState &state, GameState &gs, const Resources &res, GameObject &a, GameObject &b, SDL_FRect &aRect, SDL_FRect &bRect, SDL_FRect &cRect, float deltaTime)
{
	const auto genericResponse = [&gs, &a, &b, &aRect, &bRect, &cRect, deltaTime](float velXFactor = 0, float velYFactor = 0) {
		if (cRect.w < cRect.h)
		{
			if (a.velocity.x > 0)
			{
				// going right
				a.position.x -= cRect.w;
			}
			else if (a.velocity.x < 0)
			{
				// going left
				a.position.x += cRect.w;
			}
			a.velocity.x *= velXFactor;
		}
		else
		{
			if (a.velocity.y > 0)
			{
				// going down
				a.position.y -= cRect.h;
			}
			else if (a.velocity.y < 0)
			{
				a.position.y += cRect.h;
			}
			a.velocity.y *= velYFactor;
		}
	};

	switch (a.type)
	{
		case ObjectType::player:
		{
			switch (b.type)
			{
				case ObjectType::level:
				{
					genericResponse();
					break;
				}
				case ObjectType::enemy:
				{
					if (b.data.enemy.state != EnemyState::dead)
					{
						genericResponse(-1, -1);
					}
					break;
				}
			}
			break;
		}
		case ObjectType::bullet:
		{
			if (a.data.bullet.state == BulletState::flying)
			{
				switch (b.type)
				{
					case ObjectType::level:
					{
						genericResponse(false);
						a.velocity *= 0;

						Mix_PlayChannel(-1, res.chunkWallHit, 0);
						break;
					}
					case ObjectType::enemy:
					{
						if (b.data.enemy.state != EnemyState::dead)
						{
							b.direction = a.direction * -1;
							b.data.enemy.state = EnemyState::damaged;
							b.texture = res.enemyHitTex;
							b.currentAnimation = res.ANIM_IDX_ENEMY_HIT;
							b.data.enemy.hp--;
							b.velocity.x += 500.0f * a.direction * deltaTime; // bullet force push
							b.shouldFlash = true;
							b.flashTimer.reset();
							genericResponse(false);
							a.velocity *= 0;

							Mix_PlayChannel(-1, res.chunkShootHit, 0);
						}
						break;
					}
				}
			}
			break;
		}
		case ObjectType::enemy:
		{
			if (b.type != ObjectType::enemy)
			{
				genericResponse();
			}
			break;
		}
	}
}

void checkCollision(const SDLState &state, GameState &gs, const Resources &res, GameObject &a, GameObject &b, float deltaTime)
{
	SDL_FRect aRect{
		.x = a.position.x + a.collider.x,
		.y = a.position.y + a.collider.y,
		.w = a.collider.w, .h = a.collider.h
	};
	SDL_FRect bRect{
		.x = b.position.x + b.collider.x,
		.y = b.position.y + b.collider.y,
		.w = b.collider.w, .h = b.collider.h
	};

	SDL_FRect cRect{ 0 };
	if (SDL_GetRectIntersectionFloat(&aRect, &bRect, &cRect))
	{
		collisionResponse(state, gs, res, a, b, aRect, bRect, cRect, deltaTime);
	}
}

bool initialize(SDLState &state)
{
	bool initSuccess = true;

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
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
	SDL_SetRenderLogicalPresentation(state.renderer, LOGICAL_RES_WIDTH, LOGICAL_RES_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	// initialize audio
	if (!Mix_OpenAudio(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating audio device", state.window);
		cleanup(state);
		initSuccess = false;
	}

	return initSuccess;
}

void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	Mix_CloseAudio();
	SDL_Quit();
}
