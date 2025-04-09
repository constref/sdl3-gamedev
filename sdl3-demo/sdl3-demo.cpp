#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <format>
#include <vector>
#include <glm/glm.hpp>
#include <functional>

#include "animation.h"

using namespace std;

struct SDLState
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int width, height, logW, logH;
	bool fullscreen;

	SDLState() : window(nullptr), renderer(nullptr), fullscreen(false)
	{
		width = 1600;
		height = 900;
		logW = 512;
		logH = 288;
	}
};

enum class PlayerState
{
	idle, running, jumping
};

enum class BulletState
{
	flying, disintagrating, inactive
};

enum class EnemyState
{
	idle, damaged, dead
};

enum class ObjectType
{
	player, enemy, level, bullet
};

enum class KeyState
{
	down, up
};

struct PlayerData
{
	PlayerState state;
};

struct BulletData
{
	BulletState state;
};

struct EnemyData
{
	EnemyState state;
	int hp;
};

struct LevelData {};

union ObjectData
{
	PlayerData player;
	BulletData bullet;
	EnemyData enemy;
	LevelData level;
};

struct GameObject
{
	ObjectType type;
	glm::vec2 position, velocity, acceleration;
	SDL_FRect collider;
	SDL_Texture *texture;
	bool isGrounded;
	std::vector<Animation> animations;
	unsigned int currentAnimation;
	ObjectData data;
	float direction;
	bool shouldFlash;
	float flashTimer;

	GameObject()
	{
		type = ObjectType::level;
		data.level = LevelData();
		position = velocity = acceleration = glm::vec2(0, 0);
		collider = SDL_FRect{
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0
		};
		texture = nullptr;
		isGrounded = false;
		currentAnimation = 0;
		direction = 1;
		shouldFlash = false;
		flashTimer = 0;
	}
};

struct GameState
{
	GameObject player;
	std::vector<GameObject> objects;
	std::vector<GameObject> bullets;
	bool flipHorizontal;
	bool isShooting;
	uint64_t prevTime;
	double globalTime;
	float maxSpeed;
	float jumpForce;
	SDL_FRect mapViewport;

	GameState()
	{
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

		mapViewport = SDL_FRect{
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0,
		};
	}
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
*/

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
short map[MAP_ROWS][MAP_COLS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 3, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 3, 2, 0, 3, 0, 2, 2, 2, 2, 2, 0, 0, 0, 3, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int TILE_SIZE = 32;
const int MAP_W = MAP_COLS * TILE_SIZE;
const int MAP_H = MAP_ROWS * TILE_SIZE;

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
	SDL_Texture *bulletTex;
	SDL_Texture *bulletHitTex;

	Animation animIdle;
	Animation animRun;
	Animation animShootRun;
	Animation animShoot;
	Animation animSlide;
	Animation animSlideShoot;
	Animation animEnemy;
	Animation animBullet;
	Animation animBulletHit;

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
		bulletTex = loadTexture(state.renderer, "data/bullet.png");
		bulletHitTex = loadTexture(state.renderer, "data/bullet_hit.png");

		// setup animations
		animIdle = Animation(8, 1.6f);
		animRun = Animation(4, 0.5f);
		animShootRun = Animation(4, 0.5f);
		animShoot = Animation(4, 0.5f);
		animSlide = Animation(1, 1.0f);
		animSlideShoot = Animation(4, 0.5f);
		animEnemy = Animation(8, 1.0f);
		animBullet = Animation(4, 0.15f);
		animBulletHit = Animation(4, 0.15f);
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
		SDL_DestroyTexture(bulletTex);
		SDL_DestroyTexture(bulletHitTex);
	}
};

bool initialize(SDLState &state);
void handleInput(const SDLState &state, GameState &gs, SDL_Scancode key, KeyState keyState);
void update(const SDLState &state, GameState &gs, GameObject &obj, Resources &res, const bool *keys, float deltaTime);
void drawParalaxLayer(SDLState &state, GameState &gameState, SDL_Texture *tex, float &scrollPos, float scrollFactor, float deltaTime);
bool checkCollision(GameState &gs, GameObject &a, GameObject &b, float deltaTime);
void cleanup(SDLState &state);

bool isNearZero(float v, float epsilon = 0.0001f)
{
	return std::abs(v) < 0.0001f;
}
void setZeroIfNear(float &v)
{
	if (isNearZero(v))
	{
		v = 0;
	}
}

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
	gs.player.data.player.state = PlayerState::idle;
	gs.player.position.x = gs.mapViewport.w / 2 - spriteSize / 2;
	gs.player.collider = SDL_FRect{
		.x = 11, .y = 2, .w = 10, .h = 30
	};

	gs.player.animations = {
		res.animIdle, res.animRun, res.animShootRun, res.animShoot, res.animSlide, res.animSlideShoot
	};

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
				o.collider = SDL_FRect{
					.x = 0, .y = 0, .w = spriteSize, .h = spriteSize
				};
				return o;
			};

			switch (map[r][c])
			{
				case 1:
				{
					// ground
					GameObject o = createObject(ObjectType::level, r, c, res.groundTex);
					gs.objects.push_back(o);
					break;
				}
				case 2:
				{
					// paneling
					GameObject o = createObject(ObjectType::level, r, c, res.panelTex);
					gs.objects.push_back(o);
					break;
				}
				case 3:
				{
					// enemy
					GameObject o = createObject(ObjectType::enemy, r, c, res.enemyTex);
					o.collider = SDL_FRect{
						.x = 10, .y = 4, .w = 12, .h = spriteSize - 4
					};
					o.animations = { res.animEnemy };
					gs.objects.push_back(o);
					break;
				}
			}
		}
	}

	float bg2Scroll = 0;
	float bg3Scroll = 0;
	float bg4Scroll = 0;

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
					handleInput(state, gs, event.key.scancode, KeyState::down);
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					handleInput(state, gs, event.key.scancode, KeyState::up);
					break;
				}
			}
		}

		update(state, gs, gs.player, res, keys, deltaTime);
		// move our selected animation forward
		gs.player.animations[gs.player.currentAnimation].progress(deltaTime);

		// handle player collisions
		glm::vec2 oldPos = gs.player.position;
		gs.player.position += gs.player.velocity * deltaTime;

		for (GameObject &obj : gs.objects)
		{
			checkCollision(gs, gs.player, obj, deltaTime);
		}

		// use a sensor to check if on ground
		SDL_FRect groundSensor{ 0 };
		{
			GameObject &a = gs.player;
			groundSensor = SDL_FRect{
				.x = a.position.x + a.collider.x,
				.y = a.position.y + a.collider.y + a.collider.h,
				.w = a.collider.w, .h = 1 // using 2 pixels as collision response could push collider far enough for ground sensor to not trip
			};
			bool foundGround = false;
			for (const GameObject &o : gs.objects)
			{
				if (o.type == ObjectType::level)
				{
					SDL_FRect oRect{
						.x = o.position.x,
						.y = o.position.y,
						.w = o.collider.w,
						.h = o.collider.h
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
			if (!a.isGrounded && foundGround)
			{
				// trigger landing event
				gs.player.data.player.state = a.velocity.x == 0 ? PlayerState::idle : PlayerState::running;
			}
			a.isGrounded = foundGround;
		}

		// update bullets
		for (GameObject &bullet : gs.bullets)
		{
			if (bullet.data.bullet.state != BulletState::inactive)
			{
				bullet.position += bullet.velocity * deltaTime;
				bullet.animations[bullet.currentAnimation].progress(deltaTime);
				update(state, gs, bullet, res, keys, deltaTime);

				for (GameObject &obj : gs.objects)
				{
					checkCollision(gs, bullet, obj, deltaTime);
				}
			}
		}

		// update enemies
		for (GameObject &o : gs.objects)
		{
			if (o.type == ObjectType::enemy)
			{
				o.position += o.velocity * deltaTime;
				update(state, gs, o, res, keys, deltaTime);
			}
		}

		// finally overwrite with the resolved position (if collisions occurred)
		const float moveXDiff = gs.player.position.x - oldPos.x;
		gs.mapViewport.x += moveXDiff;

		// perform drawing commands
		SDL_RenderTexture(state.renderer, res.bg1Tex, nullptr, nullptr);

		drawParalaxLayer(state, gs, res.bg4Tex, bg4Scroll, 0.005f, deltaTime); // furthest first
		drawParalaxLayer(state, gs, res.bg3Tex, bg3Scroll, 0.05f, deltaTime);
		drawParalaxLayer(state, gs, res.bg2Tex, bg2Scroll, 0.1f, deltaTime);

		// draw the level tiles
		for (const GameObject &o : gs.objects)
		{
			if (o.type == ObjectType::level)
			{
				SDL_FRect src{
					.x = 0, .y = 0,
					.w = static_cast<float>(o.texture->w),
					.h = static_cast<float>(o.texture->h)
				};

				SDL_FRect dst{
					.x = o.position.x - gs.mapViewport.x, .y = o.position.y,
					.w = static_cast<float>(o.texture->w),
					.h = static_cast<float>(o.texture->h)
				};
				SDL_RenderTexture(state.renderer, o.texture, &src, &dst);
			}
		}

		// draw the player
		SDL_FRect src{
			.x = gs.player.animations[gs.player.currentAnimation].currentFrame() * static_cast<float>(spriteSize),
			.y = 0,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_FRect dst{
			.x = gs.player.position.x - gs.mapViewport.x,
			.y = gs.player.position.y,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_RenderTextureRotated(state.renderer, gs.player.texture, &src, &dst, 0, nullptr,
			gs.player.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

		// draw just enemies
		for (GameObject &e : gs.objects)
		{
			if (e.type == ObjectType::enemy)
			{
				e.animations[e.currentAnimation].progress(deltaTime);
				SDL_FRect src{
					.x = static_cast<float>(e.animations[e.currentAnimation].currentFrame() * spriteSize),
					.y = 0,
					.w = static_cast<float>(spriteSize),
					.h = static_cast<float>(spriteSize)
				};

				SDL_FRect dst{
					.x = e.position.x - gs.mapViewport.x,
					.y = e.position.y,
					.w = static_cast<float>(spriteSize),
					.h = static_cast<float>(spriteSize)
				};

				glm::vec2 pDir = gs.player.position - e.position; // vector pointing to player

				if (!e.shouldFlash)
				{
					SDL_RenderTextureRotated(state.renderer, e.texture, &src, &dst, 0, nullptr,
						pDir.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
				}
				else
				{
					SDL_SetTextureColorModFloat(e.texture, 3.0f, 1.5f, 1.5f);
					SDL_RenderTextureRotated(state.renderer, e.texture, &src, &dst, 0, nullptr,
						pDir.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
					SDL_SetTextureColorModFloat(e.texture, 1.0f, 1.0f, 1.0f);

					e.flashTimer += deltaTime;
					if (e.flashTimer >= 0.05f)
					{
						e.flashTimer -= 0.05f;
						e.shouldFlash = false;
					}
				}
			}
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

		// DEBUGGING
		//SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 255);
		//SDL_FRect colliderRect{
		//	.x = gs.player.position.x + gs.player.collider.x - mapViewport.x,
		//	.y = gs.player.position.y + gs.player.collider.y - mapViewport.y,
		//	.w = gs.player.collider.w, .h = gs.player.collider.h
		//};
		//SDL_RenderRect(state.renderer, &colliderRect);

		//SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
		//SDL_FRect groundRect{
		//	.x = groundSensor.x - mapViewport.x,
		//	.y = groundSensor.y - mapViewport.y,
		//	.w = static_cast<float>(groundSensor.w),
		//	.h = static_cast<float>(groundSensor.h)
		//};
		//SDL_RenderRect(state.renderer, &groundRect);


		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		gs.prevTime = nowTime;

		// show some stats
		gs.globalTime += deltaTime;
		SDL_SetWindowTitle(state.window, std::format("Runtime: {:.3f} -- {} -- {} -- G({}) B({}, ({}, {})",
			gs.globalTime, static_cast<int>(gs.player.data.player.state), gs.player.direction, gs.player.isGrounded, gs.bullets.size(),
			gs.player.velocity.x, gs.player.velocity.y).c_str());
	}

	res.cleanup();
	cleanup(state);
	return 0;
}

void handleInput(const SDLState &state, GameState &gs, SDL_Scancode key, KeyState keyState)
{
	const auto performJump = [&gs]() {
		if (gs.player.isGrounded)
		{
			gs.player.data.player.state = PlayerState::jumping;
			gs.player.velocity.y = gs.jumpForce;
			gs.player.isGrounded = false;
		}
	};

	switch (gs.player.data.player.state)
	{
		case PlayerState::idle:
		{
			if (key == SDL_SCANCODE_K && keyState == KeyState::down)
			{
				performJump();
			}
			break;
		}
		case PlayerState::running:
		{
			if (key == SDL_SCANCODE_K && keyState == KeyState::down)
			{
				if (gs.player.isGrounded)
				{
					performJump();
				}
			}
			break;
		}
		case PlayerState::jumping:
		{
			break;
		}
	}
}

void update(const SDLState &state, GameState &gs, GameObject &obj, Resources &res, const bool *keys, float deltaTime)
{
	if (obj.type == ObjectType::player)
	{
		// repeated code used for handling shooting animations
		const auto handleShooting = [&](PlayerAnimation shootAnim, SDL_Texture *shootTex, PlayerAnimation nonShootAnim, SDL_Texture *nonShootTex)
		{
			if (gs.isShooting)
			{
				gs.player.currentAnimation = static_cast<int>(shootAnim);
				gs.player.texture = shootTex;

				static double bulletTime = 0;

				if (gs.globalTime - bulletTime > 0.1)
				{
					// spawn some bullets
					const float left = -4;
					const float right = 36;
					const float t = (obj.direction + 1) / 2.0f; // -1/1 + 1 = 0/2 divided by 2.0f -> 0.0f/1.0f
					const float xOffset = left + right * t; // lerp
					GameObject b;
					b.type = ObjectType::bullet;
					b.data.bullet.state = BulletState::flying;
					b.position = gs.player.position + glm::vec2(xOffset, 32 / 2 + 1);
					int yVelRand = SDL_rand(40) - 20; // -20 to 20
					b.velocity = glm::vec2(gs.player.velocity.x + 600.0f, yVelRand) * obj.direction;
					b.texture = res.bulletTex;
					b.animations = { res.animBullet, res.animBulletHit };
					b.currentAnimation = static_cast<int>(BulletAnimation::flying);
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

					bulletTime = gs.globalTime;
				}
			}
			else
			{
				gs.player.currentAnimation = static_cast<int>(nonShootAnim);
				gs.player.texture = nonShootTex;
			}
		};

		// continuous input polling, running and/or shooting
		float currentDirection = 0;
		if (keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
			gs.flipHorizontal = true;
		}
		if (keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
			gs.flipHorizontal = false;
		}
		if (currentDirection)
		{
			gs.player.direction = currentDirection;
		}
		gs.isShooting = keys[SDL_SCANCODE_J];

		// apply velocity based movement
		gs.player.velocity += currentDirection * gs.player.acceleration * deltaTime;
		if (std::abs(gs.player.velocity.x) > gs.maxSpeed)
		{
			gs.player.velocity.x = currentDirection * gs.maxSpeed;
		}

		// handle state specifics
		switch (gs.player.data.player.state)
		{
			case PlayerState::idle:
			{
				// if play is holding a direction, move to running state
				if (currentDirection)
				{
					gs.player.data.player.state = PlayerState::running;
				}
				else
				{
					// decelerate horizontally
					if (gs.player.velocity.x)
					{
						// apply inverse force to decelerate
						const float fac = gs.player.velocity.x > 0 ? -1.5f : 1.5f;
						float decAmount = fac * gs.player.acceleration.x * deltaTime;
						// if the velocity left is less than the per-frame deceleration amount, just set to zero
						if (std::abs(gs.player.velocity.x) < decAmount)
						{
							gs.player.velocity.x = 0;
						}
						else
						{
							gs.player.velocity.x += decAmount;
						}
					}
					// standing and shooting?
					handleShooting(PlayerAnimation::shoot, res.shootTex, PlayerAnimation::idle, res.idleTex);
				}
				break;
			}
			case PlayerState::running:
			{
				// if direction is 0, then we're idling
				if (!currentDirection)
				{
					gs.player.data.player.state = PlayerState::idle;
				}
				else
				{
					// running and shooting?
					handleShooting(PlayerAnimation::shootRun, res.shootRunTex, PlayerAnimation::run, res.runTex);

					// if velocity and direction have different signs, we're sliding
					// and starting to move in the opposite direction
					if (gs.player.velocity.x * gs.player.direction < 0 && gs.player.isGrounded)
					{
						// sliding and shooting?
						handleShooting(PlayerAnimation::slideShoot, res.slideShootTex, PlayerAnimation::slide, res.slideTex);
					}
				}
				break;
			}
			case PlayerState::jumping:
			{
				handleShooting(PlayerAnimation::shootRun, res.shootRunTex, PlayerAnimation::run, res.runTex);
				break;
			}
		}

		// apply some constant gravity if not grounded
		if (!gs.player.isGrounded)
		{
			gs.player.velocity += glm::vec2(0, 500) * deltaTime;
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
				if (obj.velocity.x == 0 && obj.velocity.y == 0)
				{
					// switching to a colliding state and animation
					obj.data.bullet.state = BulletState::disintagrating;
					obj.texture = res.bulletHitTex;
					obj.currentAnimation = static_cast<int>(BulletAnimation::colliding);
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

	scrollPos -= gs.player.velocity.x * scrollFactor * deltaTime;
	if (scrollPos <= -tex->w)
	{
		scrollPos = 0;
	}
	SDL_RenderTextureTiled(state.renderer, tex, &src, 1, &dst);
}

void collisionResponse(GameState &gs, GameObject &a, GameObject &b, SDL_FRect &aRect, SDL_FRect &bRect, SDL_FRect &cRect, float deltaTime)
{
	if (a.type == ObjectType::player)
	{
		if (cRect.w < cRect.h)
		{
			if (a.velocity.x > 0)
			{
				// going right
				a.position.x = (aRect.x - a.collider.x) - cRect.w;
			}
			else if (a.velocity.x < 0)
			{
				// going left
				a.position.x = (aRect.x - a.collider.x) + cRect.w;
			}

			// bounce off enemies
			if (b.type == ObjectType::enemy)
			{
				a.velocity.x *= -1.0f;
			}
			else
			{
				a.velocity.x = 0;
			}
		}
		else
		{
			if (a.velocity.y > 0)
			{
				// going down
				a.position.y = (aRect.y - a.collider.y) - cRect.h;
			}
			else if (a.velocity.y < 0)
			{
				a.position.y = (aRect.y - a.collider.y) + cRect.h;
			}

			if (b.type == ObjectType::enemy)
			{
				a.velocity.y *= -1.0f;
			}
			else
			{
				a.velocity.y = 0;
			}
		}
	}
	else if (a.type == ObjectType::bullet)
	{
		if (cRect.w < cRect.h)
		{
			if (a.velocity.x > 0)
			{
				// going right
				a.position.x = (aRect.x - a.collider.x) - cRect.w;
			}
			else if (a.velocity.x < 0)
			{
				// going left
				a.position.x = (aRect.x - a.collider.x) + cRect.w;
			}
		}
		else
		{
			if (a.velocity.y > 0)
			{
				// going down
				a.position.y = (aRect.y - a.collider.y) - cRect.h;
			}
			else if (a.velocity.y < 0)
			{
				a.position.y = (aRect.y - a.collider.y) + cRect.h;
			}
		}
		a.velocity *= 0;
		a.acceleration *= 0;

		if (b.type == ObjectType::enemy && a.data.bullet.state == BulletState::flying)
		{
			b.data.enemy.state = EnemyState::damaged;
			b.shouldFlash = true;
			b.flashTimer = 0;
		}
	}
}

bool checkCollision(GameState &gs, GameObject &a, GameObject &b, float deltaTime)
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
		collisionResponse(gs, a, b, aRect, bRect, cRect, deltaTime);
	}
	return hasCollision;
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
