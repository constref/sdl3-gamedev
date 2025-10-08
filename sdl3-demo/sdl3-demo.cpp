#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <sstream>
#include <string>
#include <array>
#include <format>
#include <filesystem>

#include "sdlstate.h"
#include "resources.h"
#include "gameobject.h"
#include "gamestate.h"
#include "components/animationcomponent.h"
#include "components/rendercomponent.h"
#include "components/inputcomponent.h"
#include "components/physicscomponent.h"
#include "components/collisioncomponent.h"
#include "components/player/playercontrollercomponent.h"
#include "framecontext.h"
#include "inputstate.h"

using namespace std;

bool initialize(SDLState &state);
void createTiles(const SDLState &state, GameState &gs, const Resources &res);
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

	// load game assets
	Resources &res = Resources::getInstance();
	res.load(state.renderer);

	// setup game data
	GameState gs(state.logW, state.logH);
	createTiles(state, gs, res);

	//MIX_PlayAudio(nullptr, res.musicMain);

	// start the game loop
	uint64_t prevTime = SDL_GetTicks();
	float timeAccum = 0;
	bool running = true;
	InputState inputState;

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
				case SDL_EVENT_WINDOW_RESIZED:
				{
					state.width = event.window.data1;
					state.height = event.window.data2;
					break;
				}
				case SDL_EVENT_KEY_DOWN:
				{
					inputState.setKeyState(event.key.scancode, true);
					inputState.addEvent(event.key.scancode, true);
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					inputState.setKeyState(event.key.scancode, false);
					inputState.addEvent(event.key.scancode, true);
					if (event.key.scancode == SDL_SCANCODE_F2)
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

		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f;
		prevTime = nowTime;
		timeAccum += deltaTime;

		FrameContext ctx(state, gs, res, inputState, deltaTime);
		// perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		// calculate viewport position
		gs.mapViewport.x = (gs.player()->getPosition().x + TILE_SIZE / 2) - gs.mapViewport.w / 2;
		gs.mapViewport.y = res.map->mapHeight * res.map->tileHeight - gs.mapViewport.h;

		// update all objects
		for (auto &layer : gs.layers)
		{
			for (auto obj : layer)
			{
				obj->update(ctx);
			}
		}

		const int mapWPixels = res.map->mapWidth * res.map->tileWidth;
		const int mapHPixels = res.map->mapHeight * res.map->tileHeight;

		/*
		// draw background images
		SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
		drawParalaxBackground(state, gs, res.texBg4, gs.player().velocity.x,
			gs.bg4Scroll, 0.075f, deltaTime);
		drawParalaxBackground(state, gs, res.texBg3, gs.player().velocity.x,
			gs.bg3Scroll, 0.150f, deltaTime);
		drawParalaxBackground(state, gs, res.texBg2, gs.player().velocity.x,
			gs.bg2Scroll, 0.3f, deltaTime);

		// draw background tiles
		//for (GameObject &obj : gs.backgroundTiles)
		//{
		//	SDL_FRect dst{
		//		.x = obj.position.x - gs.mapViewport.x,
		//		.y = obj.position.y,
		//		.w = static_cast<float>(obj.texture->w),
		//		.h = static_cast<float>(obj.texture->h)
		//	};
		//	SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
		//}

		// draw all objects
		//for (auto &layer : gs.layers)
		//{
		//	for (GameObject &obj : layer)
		//	{
		//		drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
		//	}
		//}

		// draw bullets
		for (GameObject &bullet : gs.bullets)
		{
			if (bullet.data.bullet.state != BulletState::inactive)
			{
				drawObject(state, gs, bullet, bullet.collider.w, bullet.collider.h, deltaTime);
			}
		}

		// draw foreground tiles
		for (GameObject &obj : gs.foregroundTiles)
		{
			SDL_FRect dst{
				.x = obj.position.x - gs.mapViewport.x,
				.y = obj.position.y,
				.w = static_cast<float>(obj.texture->w),
				.h = static_cast<float>(obj.texture->h)
			};
			SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
		}

		*/
		if (gs.debugMode)
		{
			// display some debug info
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5,
				std::format("G: {} - dt: {}", gs.player()->getComponent<PhysicsComponent>()->isGrounded(), ctx.deltaTime).c_str());
		}

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
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
	//if (!MIX_Init())
	//{
	//	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating audio device", state.window);
	//	cleanup(state);
	//	initSuccess = false;
	//}

	SDL_SetWindowFullscreen(state.window, state.fullscreen);

	return initSuccess;
}

void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}

/*
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
		obj.velocity += glm::vec2(0, 500) * deltaTime * obj.gravityFactor;
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
					bullet.dynamic = true;
					bullet.gravityFactor = 0;
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

					MIX_PlayAudio(nullptr, res.audioShoot);
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
					currentDirection = playerDir.x < 0 ? -1.0f : 1.0f;
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
	bool wasGrounded = obj.grounded;
	obj.grounded = false;
	for (auto &layer : gs.layers)
	{
		for (GameObject &objB : layer)
		{
			if (&obj != &objB &&
				objB.collider.w != 0 && objB.collider.h != 0)
			{
				checkCollision(state, gs, res, obj, objB, deltaTime);
			}
		}
	}
	// collision response updates obj.grounded to new state
	if (obj.grounded && !wasGrounded)
	{
		if (obj.grounded && obj.type == ObjectType::player)
		{
			obj.data.player.state = PlayerState::running;
		}
	}
}

void collisionResponse(const SDLState &state, GameState &gs, Resources &res,
	const SDL_FRect &rectA, const SDL_FRect &rectB, const glm::vec2 &overlap,
	GameObject &objA, GameObject &objB, float deltaTime)
{
	const auto genericResponse = [&]()
	{
		// colliding on the x-axis
		if (overlap.x < overlap.y)
		{
			if (objA.position.x < objB.position.x) // from left
			{
				objA.position.x -= overlap.x;
			}
			else // from right
			{
				objA.position.x += overlap.x;
			}
			objA.velocity.x = 0;
		}
		else
		{
			if (objA.position.y < objB.position.y) // from top
			{
				objA.position.y -= overlap.y;
				objA.grounded = true;
			}
			else // from bottom
			{
				objA.position.y += overlap.y;
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
					glm::vec2 prevVel = objA.velocity;
					genericResponse();
					objA.velocity = -prevVel;
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
						MIX_PlayAudio(nullptr, res.audioShootHit);
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
							MIX_PlayAudio(nullptr, res.audioEnemyHit);
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

	glm::vec2 resolution{ 0 };
	if (intersectAABB(rectA, rectB, resolution))
	{
		// found intersection, respond accordingly
		collisionResponse(state, gs, res, rectA, rectB, resolution, a, b, deltaTime);
	}
}

*/

void createTiles(const SDLState &state, GameState &gs, const Resources &res)
{
	struct LayerVisitor
	{
		const SDLState &state;
		GameState &gs;
		const Resources &res;

		LayerVisitor(const SDLState &state, GameState &gs, const Resources &res) : state(state), gs(gs), res(res) {}

		auto createObject(int r, int c)
		{
			std::shared_ptr<GameObject> obj = std::make_shared<GameObject>();
			obj->setPosition(glm::vec2(
				c * res.map->tileWidth,
				r * res.map->tileHeight));
			return obj;
		}

		void operator()(tmx::Layer &layer) // Tile layers
		{
			std::vector<std::shared_ptr<GameObject>> newLayer;
			for (int r = 0; r < res.map->mapHeight; ++r)
			{
				for (int c = 0; c < res.map->mapWidth; ++c)
				{
					const int tGid = layer.data[r * res.map->mapWidth + c];
					if (tGid) // if not an empty slot
					{
						const auto itr = std::find_if(res.tilesetTextures.begin(), res.tilesetTextures.end(),
							[tGid](const TileSetTextures &tst) {
								return tGid >= tst.firstGid && tGid < tst.firstGid + tst.textures.size();
							});
						const TileSetTextures &tst = *itr;
						SDL_Texture *tex = tst.textures[tGid - tst.firstGid];

						auto tile = createObject(r, c);
						auto &renderComponent = tile->addComponent<RenderComponent>(res.texEnemy, TILE_SIZE, TILE_SIZE);
						renderComponent.setTexture(tex);
						// only level tiles get a collision component
						if (layer.name == "Level")
						{
							auto &collisionComponent = tile->addComponent<CollisionComponent>();
							collisionComponent.setCollider(SDL_FRect{
								.x = 0, .y = 0,
								.w = static_cast<float>(res.map->tileWidth),
								.h = static_cast<float>(res.map->tileHeight)
								});
						}
						newLayer.push_back(tile);
					}
				}
			}
			gs.layers.push_back(newLayer);
		}
		void operator()(tmx::ObjectGroup &objectGroup) // Object layers
		{
			std::vector<std::shared_ptr<GameObject>> newLayer;
			for (tmx::LayerObject &obj : objectGroup.objects)
			{
				glm::vec2 objPos(
					obj.x - res.map->tileWidth / 2,
					obj.y - res.map->tileHeight / 2);

				if (obj.type == "Player")
				{
					auto player = std::make_shared<GameObject>();
					player->setPosition(objPos);
					auto &inputComponent = player->addComponent<InputComponent>();
					auto &playerCtrlComponent = player->addComponent<PlayerControllerComponent>();
					playerCtrlComponent.setIdleAnimation(res.ANIM_PLAYER_IDLE);
					playerCtrlComponent.setIdleTexture(res.texIdle);
					playerCtrlComponent.setRunAnimation(res.ANIM_PLAYER_RUN);
					playerCtrlComponent.setRunTexture(res.texRun);
					playerCtrlComponent.setSlideAnimation(res.ANIM_PLAYER_SLIDE);
					playerCtrlComponent.setSlideTexture(res.texSlide);
					auto &physicsComponent = player->addComponent<PhysicsComponent>();
					physicsComponent.setAcceleration(glm::vec2(400, 0));
					physicsComponent.setMaxSpeed(glm::vec2(100, 300));
					auto &collisionComponent = player->addComponent<CollisionComponent>();
					collisionComponent.setDynamic(true);
					collisionComponent.setCollider(SDL_FRect{
						.x = 11, .y = 6,
						.w = 10, .h = 26
						});
					auto &animComponent = player->addComponent<AnimationComponent>(res.playerAnims);
					auto &renderComponent = player->addComponent<RenderComponent>(res.texIdle, TILE_SIZE, TILE_SIZE);
					player->initializeComponents();

					gs.playerIndex = static_cast<int>(newLayer.size());
					gs.playerLayer = static_cast<int>(gs.layers.size());
					newLayer.push_back(player);
				}
				else if (obj.type == "Enemy")
				{
					//auto enemy = std::make_shared<GameObject>();
					//enemy->setPosition(objPos);
					//auto animComponent = enemy->addComponent<AnimationComponent>(res.enemyAnims);
					//auto physicsComponent = std::make_unique<PhysicsComponent>(enemy);
					//physicsComponent->setDynamic(true);
					//physicsComponent->setAcceleration(glm::vec2(300, 0));
					//physicsComponent->setMaxSpeed(10);
					//auto renderComponent = std::make_unique<RenderComponent>(enemy, res.texEnemy, TILE_SIZE, TILE_SIZE, animComponent.get());
					//auto collisionComponent = std::make_unique<CollisionComponent>(enemy);
					//collisionComponent->setCollider(SDL_FRect{
					//	.x = 10, .y = 4, .w = 12, .h = 28
					//});

					//newLayer.push_back(enemy);
				}
			}
			gs.layers.push_back(std::move(newLayer));
		}
	};

	LayerVisitor visitor(state, gs, res);
	for (auto &layer : res.map->layers)
	{
		std::visit(visitor, layer);
	}

	//for (int r = 0; r < MAP_ROWS; r++)
	//{
	//	for (int c = 0; c < MAP_COLS; c++)
	//	{
	//		switch (layer[r][c])
	//		{
	//		case 1: // ground
	//		{
	//			GameObject o = createObject(r, c);
	//			auto renderComponent = std::make_unique<RenderComponent>(res.texGround, TILE_SIZE, TILE_SIZE);
	//			o.components.push_back(std::move(renderComponent));
	//			gs.layers[LAYER_IDX_LEVEL].push_back(std::move(o));
	//			break;
	//		}
	//		case 2: // panel
	//		{
	//			GameObject o = createObject(r, c);
	//			auto renderComponent = std::make_unique<RenderComponent>(res.texPanel, TILE_SIZE, TILE_SIZE);
	//			o.components.push_back(std::move(renderComponent));
	//			gs.layers[LAYER_IDX_LEVEL].push_back(std::move(o));
	//			break;
	//		}
	//		case 3: // enemy
	//		{
	//			gs.layers[LAYER_IDX_CHARACTERS].push_back(std::move(enemy));
	//			break;
	//		}
	//		case 4: // player
	//		{
	//			break;
	//		}
	//		case 5: // grass
	//		{
	//			GameObject o = createObject(r, c);
	//			auto renderComponent = std::make_unique<RenderComponent>(res.texGrass, TILE_SIZE, TILE_SIZE);
	//			o.components.push_back(std::move(renderComponent));
	//			gs.foregroundTiles.push_back(std::move(o));
	//			break;
	//		}
	//		case 6: // brick
	//		{
	//			GameObject o = createObject(r, c);
	//			auto renderComponent = std::make_unique<RenderComponent>(res.texBrick, TILE_SIZE, TILE_SIZE);
	//			o.components.push_back(std::move(renderComponent));
	//			gs.backgroundTiles.push_back(std::move(o));
	//			break;
	//		}
	//		}
	//	}
	//}
}

/*

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
			case PlayerState::running:
			{
				jump();
				break;
			}
		}
	}
}

void drawParalaxBackground(const SDLState &state, const GameState &gs, SDL_Texture *texture,
	float xVelocity, float &scrollPos, float scrollFactor, float deltaTime)
{
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w)
	{
		scrollPos = 0;
	}

	SDL_FRect dst{
		.x = scrollPos, .y = static_cast<float>(state.logH - texture->h),
		.w = texture->w * 2.0f,
		.h = static_cast<float>(texture->h)
	};

	SDL_RenderTextureTiled(state.renderer, texture, nullptr, 1, &dst);
}

*/
