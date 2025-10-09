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
					auto enemy = std::make_shared<GameObject>();
					enemy->setPosition(objPos);
					auto &physicsComponent = enemy->addComponent<PhysicsComponent>();
					physicsComponent.setAcceleration(glm::vec2(200, 0));
					physicsComponent.setMaxSpeed(glm::vec2(50, 300));
					auto &collisionComponent = enemy->addComponent<CollisionComponent>();
					collisionComponent.setDynamic(true);
					collisionComponent.setCollider(SDL_FRect{
						.x = 10, .y = 4, .w = 12, .h = 28
					});
					auto &animComponent = enemy->addComponent<AnimationComponent>(res.enemyAnims);
					animComponent.setAnimation(res.ANIM_ENEMY);
					auto &renderComponent = enemy->addComponent<RenderComponent>(res.texEnemy, TILE_SIZE, TILE_SIZE);
					enemy->initializeComponents();
					newLayer.push_back(enemy);
				}
			}
			gs.layers.push_back(std::move(newLayer));
		}
	};


	std::vector<std::shared_ptr<GameObject>> bgLayer;

	std::shared_ptr<GameObject> bg1 = std::make_shared<GameObject>();
	bg1->addComponent<RenderComponent>(res.texBg1, static_cast<float>(gs.mapViewport.w), static_cast<float>(gs.mapViewport.h))
		.setFollowViewport(false);
	bg1->initializeComponents();
	bgLayer.push_back(bg1);

	std::shared_ptr<GameObject> bg4 = std::make_shared<GameObject>();
	bg4->addComponent<RenderComponent>(res.texBg4, static_cast<float>(gs.mapViewport.w), static_cast<float>(gs.mapViewport.h))
		.setFollowViewport(false);
	bg4->initializeComponents();
	bgLayer.push_back(bg4);

	std::shared_ptr<GameObject> bg3 = std::make_shared<GameObject>();
	bg3->addComponent<RenderComponent>(res.texBg3, static_cast<float>(gs.mapViewport.w), static_cast<float>(gs.mapViewport.h))
		.setFollowViewport(false);
	bg3->initializeComponents();
	bgLayer.push_back(bg3);

	std::shared_ptr<GameObject> bg2 = std::make_shared<GameObject>();
	bg2->addComponent<RenderComponent>(res.texBg2, static_cast<float>(gs.mapViewport.w), static_cast<float>(gs.mapViewport.h))
		.setFollowViewport(false);
	bg2->initializeComponents();
	bgLayer.push_back(bg2);

	gs.layers.push_back(std::move(bgLayer));

	LayerVisitor visitor(state, gs, res);
	for (auto &layer : res.map->layers)
	{
		std::visit(visitor, layer);
	}
}

/*

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
