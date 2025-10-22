#include "platformer.h"

#include <gameobject.h>
#include <resources.h>
#include <sdlstate.h>
#include <world.h>
#include <components/animationcomponent.h>
#include <components/rendercomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>
#include <components/basiccameracomponent.h>

#include "components/playercontrollercomponent.h"
#include "components/weaponcomponent.h"

Platformer::Platformer()
{
	bg2Scroll = bg3Scroll = bg4Scroll = 0;
	debugMode = false;
}

bool Platformer::initialize(SDLState &state)
{
	World &world = World::getInstance();
	hRoot = world.createObject();

	Resources &res = Resources::getInstance();
	res.load(state.renderer);

	struct LayerVisitor
	{
		const SDLState &state;
		const Resources &res;
		GameObject &root;
		float tileWidth;
		float tileHeight;

		LayerVisitor(const SDLState &state, GameObject &root) : state(state), res(Resources::getInstance()), root(root)
		{
			tileWidth = static_cast<float>(res.map->tileWidth);
			tileHeight = static_cast<float>(res.map->tileHeight);
		}

		auto createObject(int r, int c)
		{
			World &world = World::getInstance();
			GHandle newObjHandle = world.createObject();
			GameObject &obj = world.getObject(newObjHandle);

			obj.setPosition(glm::vec2(
				c * res.map->tileWidth,
				r * res.map->tileHeight));
			return newObjHandle;
		}

		void operator()(tmx::Layer &layer) // Tile layers
		{
			World &world = World::getInstance();
			GHandle hLayer = world.createObject();
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

						GHandle hTile = createObject(r, c);
						GameObject &tile = world.getObject(hTile);
						auto &renderComponent = tile.addComponent<RenderComponent>(res.texEnemy, tileWidth, tileHeight);
						renderComponent.setTexture(tex);
						// only level tiles get a collision component
						if (layer.name == "Level")
						{
							auto &collisionComponent = tile.addComponent<CollisionComponent>();
							collisionComponent.setCollider(SDL_FRect{
								.x = 0, .y = 0,
								.w = static_cast<float>(tileWidth),
								.h = static_cast<float>(tileHeight)
								});
						}
						GameObject &layerObject = world.getObject(hLayer);
						layerObject.addChild(hTile);
					}
				}
			}
			root.addChild(hLayer);
		}
		void operator()(tmx::ObjectGroup &objectGroup) // Object layers
		{
			World &world = World::getInstance();
			GHandle hLayer = world.createObject();
			GameObject &layerObject = world.getObject(hLayer);

			for (tmx::LayerObject &obj : objectGroup.objects)
			{
				glm::vec2 objPos(
					obj.x - res.map->tileWidth / 2,
					obj.y - res.map->tileHeight / 2);

				if (obj.type == "Player")
				{
					GHandle hPlayer = world.createObject();
					GameObject &player = world.getObject(hPlayer);
					player.setPosition(objPos);
					auto &inputComponent = player.addComponent<InputComponent>();
					auto &playerCtrlComponent = player.addComponent<PlayerControllerComponent>();
					playerCtrlComponent.setIdleAnimation(res.ANIM_PLAYER_IDLE);
					playerCtrlComponent.setIdleTexture(res.texIdle);
					playerCtrlComponent.setRunAnimation(res.ANIM_PLAYER_RUN);
					playerCtrlComponent.setRunTexture(res.texRun);
					playerCtrlComponent.setSlideAnimation(res.ANIM_PLAYER_SLIDE);
					playerCtrlComponent.setSlideTexture(res.texSlide);
					player.addComponent<WeaponComponent>();
					auto &physicsComponent = player.addComponent<PhysicsComponent>();
					physicsComponent.setAcceleration(glm::vec2(400, 0));
					physicsComponent.setMaxSpeed(glm::vec2(100, 300));
					physicsComponent.setDynamic(true);
					auto &collisionComponent = player.addComponent<CollisionComponent>();
					collisionComponent.setCollider(SDL_FRect{
						.x = 11, .y = 6,
						.w = 10, .h = 26
						});
					auto &animComponent = player.addComponent<AnimationComponent>(res.playerAnims);
					auto &renderComponent = player.addComponent<RenderComponent>(res.texIdle, tileWidth, tileHeight);

					// we have our player, we can create the camera and set it as a target
					auto &camComponent = root.addComponent<BasicCameraComponent>(hPlayer, static_cast<float>(state.logW), static_cast<float>(state.logH));

					layerObject.addChild(hPlayer);
				}
				else if (obj.type == "Enemy")
				{
					GHandle hEnemy = world.createObject();
					GameObject &enemy = world.getObject(hEnemy);

					enemy.setPosition(objPos);
					auto &physicsComponent = enemy.addComponent<PhysicsComponent>();
					physicsComponent.setAcceleration(glm::vec2(200, 0));
					physicsComponent.setMaxSpeed(glm::vec2(50, 300));
					physicsComponent.setDynamic(true);
					auto &collisionComponent = enemy.addComponent<CollisionComponent>();
					collisionComponent.setCollider(SDL_FRect{
						.x = 10, .y = 4, .w = 12, .h = 28
						});
					auto &animComponent = enemy.addComponent<AnimationComponent>(res.enemyAnims);
					animComponent.setAnimation(res.ANIM_ENEMY);
					auto &renderComponent = enemy.addComponent<RenderComponent>(res.texEnemy, tileWidth, tileHeight);
					layerObject.addChild(hEnemy);
				}
			}
			root.addChild(hLayer);
		}
	};

	GameObject &root = world.getObject(hRoot);

	// add the background elements
	GHandle hBgLayer = world.createObject();
	GameObject &bgLayer = world.getObject(hBgLayer);

	GHandle hBG1 = world.createObject();
	GameObject &bg1 = world.getObject(hBG1);
	bg1.addComponent<RenderComponent>(res.texBg1, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(hBG1);

	GHandle hBG4 = world.createObject();
	GameObject &bg4 = world.getObject(hBG4);
	bg4.addComponent<RenderComponent>(res.texBg4, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(hBG4);

	GHandle hBG3 = world.createObject();
	GameObject &bg3 = world.getObject(hBG3);
	bg3.addComponent<RenderComponent>(res.texBg3, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(hBG3);

	GHandle hBG2 = world.createObject();
	GameObject &bg2 = world.getObject(hBG2);
	bg2.addComponent<RenderComponent>(res.texBg2, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(hBG2);

	root.addChild(hBgLayer);

	// load the map layers
	LayerVisitor visitor(state, root);
	for (auto &layer : res.map->layers)
	{
		std::visit(visitor, layer);
	}
	return true;
}
