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

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_AudioDeviceID audioDevice;
	int width, height, logW, logH;
	bool fullscreen;

	SDLState() : window(nullptr), renderer(nullptr), audioDevice(0)
	{
		width = 1600;
		height = 900;
		logW = 512;
		logH = 288;
		fullscreen = false;
	}
};

struct GameState
{
	static const size_t LAYER_IDX_LEVEL = 0;
	static const size_t LAYER_IDX_CHARACTERS = 1;
	std::array<std::vector<GameObject>, 2> objects;
	std::vector<GameObject> bullets;
	std::vector<GameObject> foreground, background;
	bool isShooting;
	uint64_t prevTime;
	double globalTime;
	float maxSpeed;
	float jumpForce;
	SDL_FRect mapViewport;
	size_t playerIndex;
	bool debugMode;

	GameState()
	{
		isShooting = false;
		prevTime = SDL_GetTicks();
		globalTime = 0;
		maxSpeed = 100;
		jumpForce = -200;
		mapViewport = SDL_FRect{
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0,
		};
		playerIndex = -1;
		debugMode = false;
	}

	GameObject &player() { return objects[GameState::LAYER_IDX_CHARACTERS][playerIndex]; }
};

enum class PlayerAnimation
{
	idle = 0,
	run = 1,
	shootRun = 2,
	shoot = 3,
	slide = 4,
	slideShoot = 5
};

enum class BulletAnimation
{
	flying = 0,
	colliding = 1
};

/*
	1 - Ground
	2 - Panel
	3 - Enemy
	4 - Player
	5 - Grass
	6 - Brick
*/

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;
const int MAP_W = MAP_COLS * TILE_SIZE;
const int MAP_H = MAP_ROWS * TILE_SIZE;
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

struct Resources
{
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

	int ANIM_IDX_PLAYER_IDLE = 0;
	int ANIM_IDX_PLAYER_RUN = 1;
	int ANIM_IDX_PLAYER_SHOOT_RUN = 2;
	int ANIM_IDX_PLAYER_SHOOT = 3;
	int ANIM_IDX_PLAYER_SLIDE = 4;
	int ANIM_IDX_PLAYER_SLIDE_SHOOT = 5;
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

		// setup animations
		playerAnims.resize(6);
		playerAnims[ANIM_IDX_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_IDX_PLAYER_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_IDX_PLAYER_SHOOT] = Animation(4, 0.5f);
		playerAnims[ANIM_IDX_PLAYER_SHOOT_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_IDX_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnims[ANIM_IDX_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
		enemyAnims.resize(3);
		enemyAnims[ANIM_IDX_ENEMY_IDLE] = Animation(8, 1.0f);
		enemyAnims[ANIM_IDX_ENEMY_HIT] = Animation(8, 1.0f);
		enemyAnims[ANIM_IDX_ENEMY_DIE] = Animation(18, 2.0f, true);
		bulletAnims.resize(2);
		bulletAnims[ANIM_IDX_BULLET] = Animation(4, 0.15f);
		bulletAnims[ANIM_IDX_BULLET_HIT] = Animation(4, 0.15f, true);
	}

	void cleanup()
	{
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
		SDL_DestroyTexture(enemyHitTex);
		SDL_DestroyTexture(enemyDieTex);
		SDL_DestroyTexture(bulletTex);
		SDL_DestroyTexture(bulletHitTex);
		SDL_DestroyTexture(grassTex);
		SDL_DestroyTexture(brickTex);

		Mix_FreeMusic(music);
		Mix_FreeChunk(chunkShoot);
		Mix_FreeChunk(chunkShootHit);
		Mix_FreeChunk(chunkWallHit);
	}
};

bool initialize(SDLState &state);
void handleInput(const SDLState &state, GameState &gs, GameObject &obj, SDL_Scancode key, bool isKeyDown);
void update(const SDLState &state, GameState &gs, GameObject &obj, Resources &res, const bool *keys, float deltaTime);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime);
void drawParalaxLayer(SDLState &state, GameState &gameState, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime);
bool checkCollision(const SDLState &state, GameState &gs, const Resources &res, GameObject &a, GameObject &b, float deltaTime);
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
	const int spriteSize = 32;
	const bool *keys = SDL_GetKeyboardState(nullptr);
	GameState gs;
	gs.mapViewport = SDL_FRect{
		.x = 0,
		.y = 0,
		.w = static_cast<float>(state.logW),
		.h = static_cast<float>(state.logH)
	};
	// load map
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
					o.position = glm::vec2(c * spriteSize, state.logH - (MAP_ROWS - r) * tex->h);
					o.velocity = glm::vec2(0, 0);
					o.acceleration = glm::vec2(0, 0);
					o.collider = SDL_FRect{
						.x = 0, .y = 0, .w = spriteSize, .h = spriteSize
					};
					return o;
				};

				switch (layer[r][c])
				{
					case 1:
					{
						// ground
						GameObject o = createObject(ObjectType::level, r, c, res.groundTex);
						gs.objects[GameState::LAYER_IDX_LEVEL].push_back(o);
						break;
					}
					case 2:
					{
						// paneling
						GameObject o = createObject(ObjectType::level, r, c, res.panelTex);
						gs.objects[GameState::LAYER_IDX_LEVEL].push_back(o);
						break;
					}
					case 3:
					{
						// enemy
						GameObject o = createObject(ObjectType::enemy, r, c, res.enemyTex);
						o.data.enemy = EnemyData();
						o.dynamic = true;
						o.collider = SDL_FRect{
							.x = 10, .y = 4, .w = 12, .h = spriteSize - 4
						};
						o.animations = res.enemyAnims;
						gs.objects[GameState::LAYER_IDX_CHARACTERS].push_back(o);
						break;
					}
					case 4:
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
						gs.objects[GameState::LAYER_IDX_CHARACTERS].push_back(o);
						gs.playerIndex = gs.objects[GameState::LAYER_IDX_CHARACTERS].size() - 1;
						break;
					}
					case 5:
					{
						GameObject o = createObject(ObjectType::foreground, r, c, res.grassTex);
						gs.foreground.push_back(o);
						break;
					}
					case 6:
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

	float bg2Scroll = 0;
	float bg3Scroll = 0;
	float bg4Scroll = 0;

	// start the game loop
	bool running = true;
	Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
	Mix_PlayMusic(res.music, -1);
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
					handleInput(state, gs, gs.player(), event.key.scancode, true);
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					switch (event.key.scancode)
					{
						case SDL_SCANCODE_F11:
						{
							SDL_SetWindowFullscreen(state.window, true);
							break;
						}
						case SDL_SCANCODE_F12:
						{
							gs.debugMode = !gs.debugMode;
							break;
						}
					}
					handleInput(state, gs, gs.player(), event.key.scancode, false);
					break;
				}
			}
		}

		for (auto &objLayer : gs.objects)
		{
			for (GameObject &objA : objLayer)
			{
				// apply some constant gravity if not grounded
				if (objA.dynamic && !objA.isGrounded)
				{
					objA.velocity += glm::vec2(0, 500) * deltaTime;
				}
				update(state, gs, objA, res, keys, deltaTime);

				if (objA.animations.size())
				{
					// move selected animation forward
					objA.animations[objA.currentAnimation].progress(deltaTime);
				}

				// collision detection and response
				objA.position += objA.velocity * deltaTime;

				for (auto &objLayer : gs.objects)
				{
					for (GameObject &objB : objLayer)
					{
						if (&objA != &objB)
						{
							checkCollision(state, gs, res, objA, objB, deltaTime);
						}
					}
				}

				// use a sensor to check if on ground
				SDL_FRect groundSensor{ 0 };
				{
					GameObject &a = objA;
					groundSensor = SDL_FRect{
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
								if (SDL_GetRectIntersectionFloat(&groundSensor, &oRect, &cRect))
								{
									if (cRect.w > cRect.h)
									{
										foundGround = true;
										continue;
									}
								}
							}
						}
					}
					if (!a.isGrounded && foundGround)
					{
						if (objA.type == ObjectType::player)
						{
							// trigger landing event
							objA.data.player.state = a.velocity.x == 0 ? PlayerState::idle : PlayerState::running;
						}
					}
					a.isGrounded = foundGround;
				}
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
		gs.mapViewport.x = (gs.player().position.x + spriteSize / 2) - gs.mapViewport.w / 2;

		// draw background/paralax images
		SDL_RenderTexture(state.renderer, res.bg1Tex, nullptr, nullptr);
		drawParalaxLayer(state, gs, res.bg4Tex, bg4Scroll, 0.005f, deltaTime); // furthest first
		drawParalaxLayer(state, gs, res.bg3Tex, bg3Scroll, 0.05f, deltaTime);
		drawParalaxLayer(state, gs, res.bg2Tex, bg2Scroll, 0.1f, deltaTime);

		// draw background objects
		for (GameObject &obj : gs.background)
		{
			drawObject(state, gs, obj, deltaTime);
		}

		// draw the game objects and entities
		for (auto &objLayer : gs.objects)
		{
			for (GameObject &obj : objLayer)
			{
				drawObject(state, gs, obj, deltaTime);
			}
		}

		// draw foreground object
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
		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		gs.prevTime = nowTime;

		// show some stats
		gs.globalTime += deltaTime;
	}

	res.cleanup();
	cleanup(state);
	return 0;
}

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime)
{
	const int spriteSize = 32;

	int currentFrame = obj.animations.size() ? obj.animations[obj.currentAnimation].currentFrame() : 0;
	SDL_FRect src{
		.x = static_cast<float>(currentFrame * spriteSize),
		.y = 0,
		.w = static_cast<float>(spriteSize),
		.h = static_cast<float>(spriteSize)
	};

	SDL_FRect dst{
		.x = obj.position.x - gs.mapViewport.x,
		.y = obj.position.y,
		.w = static_cast<float>(spriteSize),
		.h = static_cast<float>(spriteSize)
	};

	if (!obj.shouldFlash)
	{
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	}
	else
	{
		SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.5f, 1.5f);
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);

		if (obj.flashTimer.step(deltaTime))
		{
			obj.shouldFlash = false;
		}
	}

	if (gs.debugMode)
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
	const auto performJump = [&gs, &obj]() {
		if (obj.isGrounded)
		{
			obj.data.player.state = PlayerState::jumping;
			obj.velocity.y = gs.jumpForce;
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
	if (obj.type == ObjectType::player)
	{
		obj.data.player.weaponTimer.step(deltaTime);

		// repeated code used for handling shooting animations
		const auto handleShooting = [&](const int shootAnim, SDL_Texture *shootTex, const int nonShootAnim, SDL_Texture *nonShootTex)
		{
			if (gs.isShooting)
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
					b.data.bullet.state = BulletState::flying;
					b.position = obj.position + glm::vec2(xOffset, 32 / 2 + 1);
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
		gs.isShooting = keys[SDL_SCANCODE_J];

		// apply velocity based movement
		obj.velocity += currentDirection * obj.acceleration * deltaTime;
		if (std::abs(obj.velocity.x) > gs.maxSpeed)
		{
			obj.velocity.x = currentDirection * gs.maxSpeed;
		}

		// handle state specifics
		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				// if play is holding a direction, move to running state
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
					handleShooting(res.ANIM_IDX_PLAYER_SHOOT_RUN, res.shootRunTex, res.ANIM_IDX_PLAYER_RUN, res.runTex);

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
				handleShooting(res.ANIM_IDX_PLAYER_SHOOT_RUN, res.shootRunTex, res.ANIM_IDX_PLAYER_RUN, res.runTex);
				break;
			}
		}
	}
	else if (obj.type == ObjectType::bullet)
	{
		// deactivate bullets that are off-screen
		if (obj.position.x - gs.mapViewport.x < 0 ||
			obj.position.x - gs.mapViewport.x > state.logW ||
			obj.position.y - gs.mapViewport.y < 0 ||
			obj.position.y - gs.mapViewport.y > state.logH)
		{
			obj.data.bullet.state = BulletState::inactive;
		}

		switch (obj.data.bullet.state)
		{
			case BulletState::flying:
			{
				if (obj.velocity.x == 0)
				{
					obj.velocity *= 0;
					// switching to a colliding state and animation
					obj.data.bullet.state = BulletState::disintagrating;
				}
				break;
			}
			case BulletState::disintagrating:
			{
				obj.texture = res.bulletHitTex;
				obj.currentAnimation = res.ANIM_IDX_BULLET_HIT;

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
		glm::vec2 playerDir = gs.player().position - obj.position;
		const float dist = glm::length(playerDir);
		glm::vec2 dirNorm = glm::normalize(playerDir);
		switch (obj.data.enemy.state)
		{
			case EnemyState::shambling:
			{
				if (dist < 200)
				{
					obj.velocity.x = (obj.direction * 45.0f);
					if (obj.data.enemy.thinkTimer.step(deltaTime))
					{
						obj.direction = dirNorm.x < 0 ? -1 : 1;
					}
				}
				obj.texture = res.enemyTex;
				obj.currentAnimation = res.ANIM_IDX_ENEMY_IDLE;
				break;
			}
			case EnemyState::damaged:
			{
				if (obj.data.enemy.hp <= 0)
				{
					obj.texture = res.enemyDieTex;
					obj.currentAnimation = res.ANIM_IDX_ENEMY_DIE;
					obj.data.enemy.state = EnemyState::dead;
				}
				else
				{
					obj.texture = res.enemyHitTex;
					obj.currentAnimation = res.ANIM_IDX_ENEMY_HIT;

					Timer &dmgTimer = obj.data.enemy.dmgTimer;
					if (dmgTimer.step(deltaTime))
					{
						obj.data.enemy.state = EnemyState::shambling;
					}
				}
				break;
			}
			case EnemyState::dead:
			{
				obj.velocity.x = 0;
				break;
			}
		}
	}
}

void drawParalaxLayer(SDLState &state, GameState &gs, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime)
{
	SDL_FRect src{
		.x = 0, .y = 0,
		.w = static_cast<float>(tex->w),
		.h = static_cast<float>(tex->h)
	};
	SDL_FRect dst{
		.x = scrollPos, .y = 10,
		.w = static_cast<float>(tex->w * 2),
		.h = static_cast<float>(tex->h)
	};

	scrollPos -= gs.player().velocity.x * scrollFactor * deltaTime;
	if (scrollPos <= -tex->w)
	{
		scrollPos = 0;
	}
	SDL_RenderTextureTiled(state.renderer, tex, &src, 1, &dst);
}

void collisionResponse(const SDLState &state, GameState &gs, const Resources &res, GameObject &a, GameObject &b, SDL_FRect &aRect, SDL_FRect &bRect, SDL_FRect &cRect, float deltaTime)
{
	const auto genericResponse = [&gs, &a, &b, &aRect, &bRect, &cRect, deltaTime](bool shouldFlash = false, float velXFactor = 0, float velYFactor = 0) {
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
		else if (cRect.w == cRect.h)
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
		a.shouldFlash = shouldFlash;
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
						genericResponse(true, -1, -1);
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
						a.data.bullet.state = BulletState::disintagrating;

						Mix_PlayChannel(-1, res.chunkWallHit, 0);
						break;
					}
					case ObjectType::enemy:
					{
						if (b.data.enemy.state != EnemyState::dead)
						{
							b.direction = a.direction * -1;
							b.data.enemy.state = EnemyState::damaged;
							b.shouldFlash = true;
							b.flashTimer.reset();
							b.data.enemy.hp--;

							b.velocity.x += glm::normalize(a.velocity).x * 25;
							a.velocity *= 0;
							a.data.bullet.state = BulletState::disintagrating;
							genericResponse(false);

							Mix_PlayChannel(-1, res.chunkShootHit, 0);
						}
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

bool checkCollision(const SDLState &state, GameState &gs, const Resources &res, GameObject &a, GameObject &b, float deltaTime)
{
	bool hasCollision = false;

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
		hasCollision = true;
		collisionResponse(state, gs, res, a, b, aRect, bRect, cRect, deltaTime);
	}
	return hasCollision;
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
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

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
