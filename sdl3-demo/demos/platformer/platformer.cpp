#include "platformer.h"

#include <node.h>
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
#include <prototypeinstancer.h>
#include <messaging/events.h>
#include <resourceloader.h>

#include "systems/playercontrolsystem.h"
#include "systems/basiccamerasystem.h"
#include "systems/weaponsystem.h"
#include "systems/projectilesystem.h"
#include "systems/damagesystem.h"
#include "systems/enemysystem.h"
#include "components/playercontrollercomponent.h"
#include "components/weaponcomponent.h"
#include "components/healthcomponent.h"
#include "components/enemycomponent.h"

#include <filesystem>

Platformer::Platformer()
{
	bg2Scroll = bg3Scroll = bg4Scroll = 0;
	debugMode = false;
}

bool Platformer::initialize(Services &services, SDLState &state)
{
	// start up gameplay systems
	services.compSys().registerSystem(std::make_unique<PlayerControlSystem>(services));
	services.compSys().registerSystem(std::make_unique<WeaponSystem>(services));
	services.compSys().registerSystem(std::make_unique<ProjectileSystem>(services));
	services.compSys().registerSystem(std::make_unique<BasicCameraSystem>(services));
	services.compSys().registerSystem(std::make_unique<DamageSystem>(services));
	services.compSys().registerSystem(std::make_unique<EnemySystem>(services));

	return true;
}

void Platformer::start(Services &services, SDLState &state)
{
	World &world = services.world();
	setRoot(world.createNode());

	// TODO: Make it more obvious this is a throwaway object, it holds onto pointers to systems
	ResourceLoader loader(services);

	animPlayerIdle = loader.createAnimation(8, 1.6f);
	animPlayerRun = loader.createAnimation(4, 0.5f);
	animPlayerJump = loader.createAnimation(4, 1.0f);
	animPlayerSlide = loader.createAnimation(1, 1.0f);
	animPlayerShoot = loader.createAnimation(4, 0.5f);
	animPlayerSlideShoot = loader.createAnimation(4, 0.5f);
	animBulletMoving = loader.createAnimation(4, 0.01f);
	animBulletHit = loader.createAnimation(4, 0.15f);
	animEnemy = loader.createAnimation(8, 1.0f);
	animEnemyHit = loader.createAnimation(8, 1.0f);
	animEnemyDie = loader.createAnimation(18, 2.0f);

	//MIX_Audio *loadAudio(const std::string &filepath)
	//{
	//	MIX_Audio* audio = MIX_LoadAudio(nullptr, filepath.c_str(), true);
	//	audioBuffers.push_back(audio);
	//	//MIX_VolumeChunk(chunk, MIX_MAX_VOLUME / 2);
	//	return audio;
	//}

	const std::string prefix = "data/";
	texIdle = loader.loadTexture(prefix + "idle.png", true);
	texRun = loader.loadTexture(prefix + "run.png", true);
	texSlide = loader.loadTexture(prefix + "slide.png", true);
	texBrick = loader.loadTexture(prefix + "tiles/brick.png", true);
	texGrass = loader.loadTexture(prefix + "tiles/grass.png", true);
	texGround = loader.loadTexture(prefix + "tiles/ground.png", true);
	texPanel = loader.loadTexture(prefix + "tiles/panel.png", true);
	texBg1 = loader.loadTexture(prefix + "bg/bg_layer1.png", true);
	texBg2 = loader.loadTexture(prefix + "bg/bg_layer2.png", true);
	texBg3 = loader.loadTexture(prefix + "bg/bg_layer3.png", true);
	texBg4 = loader.loadTexture(prefix + "bg/bg_layer4.png", true);
	texBullet = loader.loadTexture(prefix + "bullet.png", true);
	texBulletHit = loader.loadTexture(prefix + "bullet_hit.png", true);
	texShoot = loader.loadTexture(prefix + "shoot.png", true);
	texRunShoot = loader.loadTexture(prefix + "shoot_run.png", true);
	texSlideShoot = loader.loadTexture(prefix + "slide_shoot.png", true);
	texEnemy = loader.loadTexture(prefix + "enemy.png", true);
	texEnemyHit = loader.loadTexture(prefix + "enemy_hit.png", true);
	texEnemyDie = loader.loadTexture(prefix + "enemy_die.png", true);

		//audioShoot = loadAudio(prefix + "audio/shoot.wav");
		//audioShootHit = loadAudio(prefix + "audio/wall_hit.wav");
		//audioEnemyHit = loadAudio(prefix + "audio/shoot_hit.wav");
		//musicMain = loadAudio(prefix + "audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");

	// load the map XML and preload image(s)
	map = tmx::loadMap(prefix + "maps/largemap.tmx");
	for (tmx::TileSet &tileSet : map->tileSets)
	{
		TileSetTextures tst;
		tst.firstGid = tileSet.firstgid;
		tst.textures.reserve(tileSet.tiles.size());

		for (tmx::Tile &tile : tileSet.tiles)
		{
			const std::string imagePath = prefix + "tiles/" + std::filesystem::path(tile.image.source).filename().string();
			tst.textures.push_back(loader.loadTexture(imagePath, true));
		}
		tilesetTextures.push_back(std::move(tst));
	}

	Node &root = world.getNode(getRoot());

	// add the background elements
	NodeHandle hBgLayer = world.createNode();
	Node &bgLayer = world.getNode(hBgLayer);

	NodeHandle hBG1 = world.createNode();
	Node &bg1 = world.getNode(hBG1);
	auto &spriteComp1 = services.compSys().addComponent<SpriteComponent>(bg1, texBg1, static_cast<float>(state.logW), static_cast<float>(state.logH));
	spriteComp1.setFollowViewport(false);
	spriteComp1.setLayerIndex(-13);
	bgLayer.addChild(bg1);

	NodeHandle hBG4 = world.createNode();
	Node &bg4 = world.getNode(hBG4);
	auto &bg4Sc = services.compSys().addComponent<SpriteComponent>(bg4, texBg4, static_cast<float>(state.logW), static_cast<float>(state.logH));
	bg4Sc.setFollowViewport(false);
	bg4Sc.setParalaxFactor(0.05f);
	bg4Sc.setLayerIndex(-12);
	bgLayer.addChild(bg4);

	NodeHandle hBG3 = world.createNode();
	Node &bg3 = world.getNode(hBG3);
	auto &bg3Sc = services.compSys().addComponent<SpriteComponent>(bg3, texBg3, static_cast<float>(state.logW), static_cast<float>(state.logH));
	bg3Sc.setFollowViewport(false);
	bg3Sc.setParalaxFactor(0.1f);
	bg3Sc.setLayerIndex(-11);
	bgLayer.addChild(bg3);

	NodeHandle hBG2 = world.createNode();
	Node &bg2 = world.getNode(hBG2);
	auto &bg2Sc = services.compSys().addComponent<SpriteComponent>(bg2, texBg2, static_cast<float>(state.logW), static_cast<float>(state.logH));
	bg2Sc.setFollowViewport(false);
	bg2Sc.setParalaxFactor(0.2f);
	bg2Sc.setLayerIndex(-10);
	bgLayer.addChild(bg2);

	root.addChild(bgLayer);

	// load the map layers
	for (int idx = -map->layers.size(); auto &layer : map->layers)
	{
		switch (layer.index())
		{
			case 0:
			{
				processLayer(root, services, state, std::get<tmx::Layer>(layer), idx++);
				break;
			}
			case 1:
			{
				processLayer(root, services, state, std::get<tmx::ObjectGroup>(layer), idx++);
				break;
			}
		}
	}
}

auto Platformer::createObject(Services &services, int r, int c)
{
	World &world = services.world();
	NodeHandle newObjHandle = world.createNode();
	Node &obj = world.getNode(newObjHandle);

	obj.setPosition(glm::vec3(
		c * map->tileWidth,
		r * map->tileHeight,
		0));
	return newObjHandle;
}
void Platformer::processLayer(Node &root, Services &services, SDLState &state, tmx::Layer &layer, int index) // Tile layers
{
	World &world = services.world();
	NodeHandle hLayer = world.createNode();
	Node &layerObject = world.getNode(hLayer);

	for (int r = 0; r < map->mapHeight; ++r)
	{
		for (int c = 0; c < map->mapWidth; ++c)
		{
			const int tGid = layer.data[r * map->mapWidth + c];
			if (tGid) // if not an empty slot
			{
				const auto itr = std::find_if(tilesetTextures.begin(), tilesetTextures.end(),
					[tGid](const TileSetTextures &tst) {
					return tGid >= tst.firstGid && tGid < tst.firstGid + tst.textures.size();
				});
				const TileSetTextures &tst = *itr;
				ResourceId tex = tst.textures[tGid - tst.firstGid];

				NodeHandle hTile = createObject(services, r, c);
				Node &tile = world.getNode(hTile);
				tile.setTag(1);
				auto &spriteComponent = services.compSys().addComponent<SpriteComponent>(tile, texEnemy, map->tileWidth, map->tileHeight);
				spriteComponent.setTexture(tex);
				spriteComponent.setLayerIndex(index);

				// only level tiles get a collision component
				if (layer.name == "Level")
				{
					auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(tile);
					collisionComponent.setCollider(Collider{
						.x = 0, .y = 0,
						.w = static_cast<float>(map->tileWidth),
						.h = static_cast<float>(map->tileHeight)
						});
				}
				Node &layerObject = world.getNode(hLayer);
				layerObject.addChild(tile);
			}
		}
	}
	root.addChild(layerObject);
}
void Platformer::processLayer(Node &root, Services &services, SDLState &state,  tmx::ObjectGroup &objectGroup, int index) // Object layers
{
	World &world = services.world();
	NodeHandle hLayer = world.createNode();
	Node &layerObject = world.getNode(hLayer);

	for (tmx::LayerObject &obj : objectGroup.objects)
	{
		glm::vec3 objPos(
			obj.x - map->tileWidth / 2,
			obj.y - map->tileHeight / 2,
			0);

		if (obj.type == "Player")
		{
			NodeHandle hPlayer = world.createNode();
			Node &player = world.getNode(hPlayer);
			player.setTag(2);
			player.setPosition(objPos);
			auto &inputComponent = services.compSys().addComponent<InputComponent>(player, hPlayer);
			auto &ctrlComp = services.compSys().addComponent<PlayerControllerComponent>(player);
			ctrlComp.setIdleAnimation(animPlayerIdle);
			ctrlComp.setIdleTexture(texIdle);
			ctrlComp.setRunAnimation(animPlayerRun);
			ctrlComp.setRunTexture(texRun);
			ctrlComp.setJumpAnimation(animPlayerJump);
			ctrlComp.setJumpTexture(texRun);
			ctrlComp.setSlideAnimation(animPlayerSlide);
			ctrlComp.setSlideTexture(texSlide);
			ctrlComp.setSlideShootAnimation(animPlayerSlideShoot);
			ctrlComp.setSlideShootTexture(texSlideShoot);
			ctrlComp.setShootAnimation(animPlayerShoot);
			ctrlComp.setShootTexture(texShoot);
			ctrlComp.setRunShootAnimation(animPlayerRun);
			ctrlComp.setRunShootTexture(texRunShoot);

			// setup the player's weapon
			WeaponComponent &wpnComp = services.compSys().addComponent<WeaponComponent>(player);
			wpnComp.animProjectile = animBulletMoving;
			wpnComp.animProjectileHit = animBulletHit;
			wpnComp.texProjectile = texBullet;
			wpnComp.texProjectileHit = texBulletHit;

			auto &physicsComponent = services.compSys().addComponent<PhysicsComponent>(player);
			physicsComponent.setAcceleration(glm::vec3(800, 0, 0));
			physicsComponent.setMaxSpeed(glm::vec3(100, 300, 0));
			auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(player);
			collisionComponent.setCollider(Collider{
				.x = 11, .y = 6,
				.w = 10, .h = 26
				});
			services.compSys().addComponent<AnimationComponent>(player);
			services.eventQueue().enqueue<AnimationPlayEvent>(hPlayer, 0, animPlayerIdle, texIdle, AnimationPlaybackMode::continuous);
			auto &spriteComp = services.compSys().addComponent<SpriteComponent>(player, texIdle, map->tileWidth, map->tileHeight);
			spriteComp.setLayerIndex(index);
			auto &camComp = services.compSys().addComponent<BasicCameraComponent>(player, glm::vec2(state.logW, state.logH), map->tileWidth, map->tileHeight, map->mapWidth, map->mapHeight);

			layerObject.addChild(player);
		}
		else if (obj.type == "Enemy")
		{
			NodeHandle hEnemy = world.createNode();
			Node &enemy = world.getNode(hEnemy);
			enemy.setTag(3);

			enemy.setPosition(objPos);
			auto &physicsComponent = services.compSys().addComponent<PhysicsComponent>(enemy);
			physicsComponent.setAcceleration(glm::vec3(200, 0, 0));
			physicsComponent.setMaxSpeed(glm::vec3(50, 300, 0));
			auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(enemy);
			collisionComponent.setCollider(Collider{
				.x = 10, .y = 4, .w = 12, .h = 28
			});
			services.compSys().addComponent<HealthComponent>(enemy, 300);
			auto &animComponent = services.compSys().addComponent<AnimationComponent>(enemy);
			services.eventQueue().enqueue<AnimationPlayEvent>(hEnemy, 0, animEnemy, texEnemy, AnimationPlaybackMode::continuous);

			auto &spriteComp = services.compSys().addComponent<SpriteComponent>(enemy, texEnemy, map->tileWidth, map->tileHeight);
			spriteComp.setLayerIndex(index);
			auto &enemyComponent = services.compSys().addComponent<EnemyComponent>(enemy, EnemyType::creeper);
			enemyComponent.deathAnimation = animEnemyDie;
			enemyComponent.deathTexture = texEnemyDie;

			layerObject.addChild(enemy);
		}
	}
	root.addChild(layerObject);
}
