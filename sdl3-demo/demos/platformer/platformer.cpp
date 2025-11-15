#include "platformer.h"

#include <node.h>
#include <resources.h>
#include <sdlstate.h>
#include <world.h>
#include <components/animationcomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>
#include <components/basiccameracomponent.h>
#include <components/spritecomponent.h>
#include <systems/systemregistry.h>
#include <componentsystems.h>

#include "systems/playercontrolsystem.h"
#include "systems/basiccamerasystem.h"
#include "systems/weaponsystem.h"
#include "systems/projectilesystem.h"
#include "components/playercontrollercomponent.h"
#include "components/weaponcomponent.h"
#include "components/healthcomponent.h"
#include "components/enemycomponent.h"

Platformer::Platformer()
{
	bg2Scroll = bg3Scroll = bg4Scroll = 0;
	debugMode = false;
}

bool Platformer::initialize(Services &services, SDLState &state)
{
	World &world = services.world();
	hRoot = world.createNode();

	Resources &res = Resources::get();
	res.load(state.renderer);

	services.compSys().registerSystem(std::make_unique<PlayerControlSystem>(services));
	services.compSys().registerSystem(std::make_unique<WeaponSystem>(services));
	services.compSys().registerSystem(std::make_unique<ProjectileSystem>(services));
	services.compSys().registerSystem(std::make_unique<BasicCameraSystem>(services, glm::vec2(state.logW, state.logH)));

	struct LayerVisitor
	{
		const SDLState &state;
		const Resources &res;
		Services &services;
		Node &root;
		float tileWidth;
		float tileHeight;

		LayerVisitor(Services &services, const SDLState &state, Node &root) : services(services), state(state), res(Resources::get()), root(root)
		{
			tileWidth = static_cast<float>(res.map->tileWidth);
			tileHeight = static_cast<float>(res.map->tileHeight);
		}

		auto createObject(int r, int c)
		{
			World &world = services.world();
			NodeHandle newObjHandle = world.createNode();
			Node &obj = world.getNode(newObjHandle);

			obj.setPosition(glm::vec2(
				c * res.map->tileWidth,
				r * res.map->tileHeight));
			return newObjHandle;
		}

		void operator()(tmx::Layer &layer) // Tile layers
		{
			World &world = services.world();
			NodeHandle hLayer = world.createNode();
			Node &layerObject = world.getNode(hLayer);

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

						NodeHandle hTile = createObject(r, c);
						Node &tile = world.getNode(hTile);
						tile.setTag(1);
						auto &renderComponent = services.compSys().addComponent<SpriteComponent>(tile, res.texEnemy, tileWidth, tileHeight);
						renderComponent.setTexture(tex);
						// only level tiles get a collision component
						if (layer.name == "Level")
						{
							auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(tile);
							collisionComponent.setCollider(SDL_FRect{
								.x = 0, .y = 0,
								.w = static_cast<float>(tileWidth),
								.h = static_cast<float>(tileHeight)
								});
						}
						Node &layerObject = world.getNode(hLayer);
						layerObject.addChild(tile);
					}
				}
			}
			root.addChild(layerObject);
		}
		void operator()(tmx::ObjectGroup &objectGroup) // Object layers
		{
			World &world = services.world();
			NodeHandle hLayer = world.createNode();
			Node &layerObject = world.getNode(hLayer);

			for (tmx::LayerObject &obj : objectGroup.objects)
			{
				glm::vec2 objPos(
					obj.x - res.map->tileWidth / 2,
					obj.y - res.map->tileHeight / 2);

				if (obj.type == "Player")
				{
					NodeHandle hPlayer = world.createNode();
					Node &player = world.getNode(hPlayer);
					player.setTag(2);
					player.setPosition(objPos);
					auto &inputComponent = services.compSys().addComponent<InputComponent>(player, hPlayer);
					auto &ctrlComp = services.compSys().addComponent<PlayerControllerComponent>(player);
					ctrlComp.setIdleAnimation(res.ANIM_PLAYER_IDLE);
					ctrlComp.setIdleTexture(res.texIdle);
					ctrlComp.setRunAnimation(res.ANIM_PLAYER_RUN);
					ctrlComp.setRunTexture(res.texRun);
					ctrlComp.setJumpAnimation(res.ANIM_PLAYER_JUMP);
					ctrlComp.setJumpTexture(res.texRun);
					ctrlComp.setSlideAnimation(res.ANIM_PLAYER_SLIDE);
					ctrlComp.setSlideTexture(res.texSlide);
					ctrlComp.setSlideShootAnimation(res.ANIM_PLAYER_SLIDE_SHOOT);
					ctrlComp.setSlideShootTexture(res.texSlideShoot);
					ctrlComp.setShootAnimation(res.ANIM_PLAYER_SHOOT);
					ctrlComp.setShootTexture(res.texShoot);
					ctrlComp.setRunShootAnimation(res.ANIM_PLAYER_RUN);
					ctrlComp.setRunShootTexture(res.texRunShoot);
					services.compSys().addComponent<WeaponComponent>(player);
					auto &physicsComponent = services.compSys().addComponent<PhysicsComponent>(player);
					physicsComponent.setAcceleration(glm::vec2(800, 0));
					physicsComponent.setMaxSpeed(glm::vec2(100, 300));
					physicsComponent.setDynamic(true);
					auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(player);
					collisionComponent.setCollider(SDL_FRect{
						.x = 11, .y = 6,
						.w = 10, .h = 26
						});
					auto &animComponent = services.compSys().addComponent<AnimationComponent>(player, res.playerAnims);
					auto &renderComponent = services.compSys().addComponent<SpriteComponent>(player, res.texIdle, tileWidth, tileHeight);
					services.compSys().addComponent<BasicCameraComponent>(player);

					layerObject.addChild(player);
				}
				else if (obj.type == "Enemy")
				{
					NodeHandle hEnemy = world.createNode();
					Node &enemy = world.getNode(hEnemy);
					enemy.setTag(3);

					enemy.setPosition(objPos);
					auto &physicsComponent = services.compSys().addComponent<PhysicsComponent>(enemy);
					physicsComponent.setAcceleration(glm::vec2(200, 0));
					physicsComponent.setMaxSpeed(glm::vec2(50, 300));
					physicsComponent.setDynamic(true);
					auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(enemy);
					collisionComponent.setCollider(SDL_FRect{
						.x = 10, .y = 4, .w = 12, .h = 28
						});
					services.compSys().addComponent<HealthComponent>(enemy, 300);
					auto &animComponent = services.compSys().addComponent<AnimationComponent>(enemy, res.enemyAnims);
					animComponent.setAnimation(res.ANIM_ENEMY);
					auto &renderComponent = services.compSys().addComponent<SpriteComponent>(enemy, res.texEnemy, tileWidth, tileHeight);
					services.compSys().addComponent<EnemyComponent>(enemy, EnemyType::creeper);

					layerObject.addChild(enemy);
				}
			}
			root.addChild(layerObject);
		}
	};

	Node &root = world.getNode(hRoot);

	// add the background elements
	NodeHandle hBgLayer = world.createNode();
	Node &bgLayer = world.getNode(hBgLayer);

	NodeHandle hBG1 = world.createNode();
	Node &bg1 = world.getNode(hBG1);
	services.compSys().addComponent<SpriteComponent>(bg1, res.texBg1, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(bg1);

	NodeHandle hBG4 = world.createNode();
	Node &bg4 = world.getNode(hBG4);
	services.compSys().addComponent<SpriteComponent>(bg4, res.texBg4, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(bg4);

	NodeHandle hBG3 = world.createNode();
	Node &bg3 = world.getNode(hBG3);
	services.compSys().addComponent<SpriteComponent>(bg3, res.texBg3, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(bg3);

	NodeHandle hBG2 = world.createNode();
	Node &bg2 = world.getNode(hBG2);
	services.compSys().addComponent<SpriteComponent>(bg2, res.texBg2, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(bg2);

	root.addChild(bgLayer);

	// load the map layers
	LayerVisitor visitor(services, state, root);
	for (auto &layer : res.map->layers)
	{
		std::visit(visitor, layer);
	}
	return true;
}

void Platformer::cleanup()
{
	Resources::get().unload();
}
