#pragma once

#include <gameobject.h>
#include <components/animationcomponent.h>
#include <components/rendercomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>
#include <components/basiccameracomponent.h>

#include "components/playercontrollercomponent.h"

class Platformer
{
	std::shared_ptr<GameObject> root;
	std::shared_ptr<GameObject> playerObj;

	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

public:
	Platformer()
	{
		root = std::make_shared<GameObject>();
		bg2Scroll = bg3Scroll = bg4Scroll = 0;
		debugMode = false;
	}

	std::shared_ptr<GameObject> &player() { return playerObj; }

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

	auto &getRoot()
	{
		return root;
	}

	void createTiles(const SDLState &state)
	{
		Resources &res = Resources::getInstance();
		res.load(state.renderer);

		struct LayerVisitor
		{
			const SDLState &state;
			const Resources &res;
			std::shared_ptr<GameObject> root;
			float tileWidth;
			float tileHeight;

			LayerVisitor(const SDLState &state, std::shared_ptr<GameObject> root) : state(state), root(root), res(Resources::getInstance())
			{
				tileWidth = static_cast<float>(res.map->tileWidth);
				tileHeight = static_cast<float>(res.map->tileHeight);
			}

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
				std::shared_ptr<GameObject> layerObject = std::make_shared<GameObject>();
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
							auto &renderComponent = tile->addComponent<RenderComponent>(res.texEnemy, tileWidth, tileHeight);
							renderComponent.setTexture(tex);
							// only level tiles get a collision component
							if (layer.name == "Level")
							{
								auto &collisionComponent = tile->addComponent<CollisionComponent>();
								collisionComponent.setCollider(SDL_FRect{
									.x = 0, .y = 0,
									.w = static_cast<float>(tileWidth),
									.h = static_cast<float>(tileHeight)
									});
							}
							layerObject->addChild(tile);
						}
					}
				}
				root->addChild(layerObject);
			}
			void operator()(tmx::ObjectGroup &objectGroup) // Object layers
			{
				std::shared_ptr<GameObject> layerObject = std::make_shared<GameObject>();
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
						auto &renderComponent = player->addComponent<RenderComponent>(res.texIdle, tileWidth, tileHeight);
						//player->initializeComponents();

						// we have our player, we can create the camera and set it as a target
						auto &camComponent = root->addComponent<BasicCameraComponent>(player, static_cast<float>(state.logW), static_cast<float>(state.logH));

						layerObject->addChild(player);
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
						auto &renderComponent = enemy->addComponent<RenderComponent>(res.texEnemy, tileWidth, tileHeight);
						//enemy->initializeComponents();
						layerObject->addChild(enemy);
					}
				}
				root->addChild(layerObject);
			}
		};

		// add the background elements
		auto bgLayer = std::make_shared<GameObject>();

		std::shared_ptr<GameObject> bg1 = std::make_shared<GameObject>();
		bg1->addComponent<RenderComponent>(res.texBg1, static_cast<float>(state.logW), static_cast<float>(state.logH))
			.setFollowViewport(false);
		//bg1->initializeComponents();
		bgLayer->addChild(bg1);

		std::shared_ptr<GameObject> bg4 = std::make_shared<GameObject>();
		bg4->addComponent<RenderComponent>(res.texBg4, static_cast<float>(state.logW), static_cast<float>(state.logH))
			.setFollowViewport(false);
		//bg4->initializeComponents();
		bgLayer->addChild(bg4);

		std::shared_ptr<GameObject> bg3 = std::make_shared<GameObject>();
		bg3->addComponent<RenderComponent>(res.texBg3, static_cast<float>(state.logW), static_cast<float>(state.logH))
			.setFollowViewport(false);
		//bg3->initializeComponents();
		bgLayer->addChild(bg3);

		std::shared_ptr<GameObject> bg2 = std::make_shared<GameObject>();
		bg2->addComponent<RenderComponent>(res.texBg2, static_cast<float>(state.logW), static_cast<float>(state.logH))
			.setFollowViewport(false);
		//bg2->initializeComponents();
		bgLayer->addChild(bg2);

		root->addChild(bgLayer);

		// load the map layers
		LayerVisitor visitor(state, root);
		for (auto &layer : res.map->layers)
		{
			std::visit(visitor, layer);
		}
	}
};
