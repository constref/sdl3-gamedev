#pragma once

#include <components/animationcomponent.h>
#include <components/rendercomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

#include "components/playercontrollercomponent.h"

class Platformer
{
	GameState gs;

public:
	Platformer() : gs(512, 288)
	{
	}

	bool initialize(SDLState &state)
	{
		createTiles(state);
		return true;
	}

	void cleanup()
	{
	}

	void onStart()
	{
	}

	void update(const FrameContext &ctx)
	{
	}

	void createTiles(const SDLState &state)
	{
		Resources &res = Resources::getInstance();
		res.load(state.renderer);

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

		// add the background elements
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

		// load the map layers
		LayerVisitor visitor(state, gs, res);
		for (auto &layer : res.map->layers)
		{
			std::visit(visitor, layer);
		}
	}
};
