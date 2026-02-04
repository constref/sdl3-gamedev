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
#include <systems/spriteanimationsystem.h>

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
	World &world = services.world();
	hRoot = world.createNode();

	SpriteAnimationSystem *animSys = services.compSys().getSystemRegistry().getSystem<SpriteAnimationSystem>();
	animBulletMoving = animSys->createAnimation(4, 0.01f);
	animBulletHit = animSys->createAnimation(4, 0.15f);

	playerAnims.resize(6);
	playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
	playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
	playerAnims[ANIM_PLAYER_JUMP] = Animation(4, 1.0f);
	playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
	playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
	playerAnims[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
	//bulletAnims.resize(2);
	//bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.01f);
	//bulletAnims[ANIM_BULLET_HIT] = Animation(4, 0.15f);
	enemyAnims.resize(3);
	enemyAnims[ANIM_ENEMY] = Animation(8, 1.0f);
	enemyAnims[ANIM_ENEMY_HIT] = Animation(8, 1.0f);
	enemyAnims[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

	//MIX_Audio *loadAudio(const std::string &filepath)
	//{
	//	MIX_Audio* audio = MIX_LoadAudio(nullptr, filepath.c_str(), true);
	//	audioBuffers.push_back(audio);
	//	//MIX_VolumeChunk(chunk, MIX_MAX_VOLUME / 2);
	//	return audio;
	//}

	VulkanRenderSystem *renderSys = services.compSys().getSystemRegistry().getSystem<VulkanRenderSystem>();

	const std::string prefix = "data/";
	texIdle = renderSys->loadTexture(prefix + "idle.png");
	texRun = renderSys->loadTexture(prefix + "run.png");
	texSlide = renderSys->loadTexture(prefix + "slide.png");
	texBrick = renderSys->loadTexture(prefix + "tiles/brick.png");
	texGrass = renderSys->loadTexture(prefix + "tiles/grass.png");
	texGround = renderSys->loadTexture(prefix + "tiles/ground.png");
	texPanel = renderSys->loadTexture(prefix + "tiles/panel.png");
	texBg1 = renderSys->loadTexture(prefix + "bg/bg_layer1.png");
	texBg2 = renderSys->loadTexture(prefix + "bg/bg_layer2.png");
	texBg3 = renderSys->loadTexture(prefix + "bg/bg_layer3.png");
	texBg4 = renderSys->loadTexture(prefix + "bg/bg_layer4.png");
	texBullet = renderSys->loadTexture(prefix + "bullet.png");
	texBulletHit = renderSys->loadTexture(prefix + "bullet_hit.png");
	texShoot = renderSys->loadTexture(prefix + "shoot.png");
	texRunShoot = renderSys->loadTexture(prefix + "shoot_run.png");
	texSlideShoot = renderSys->loadTexture(prefix + "slide_shoot.png");
	texEnemy = renderSys->loadTexture(prefix + "enemy.png");
	texEnemyHit = renderSys->loadTexture(prefix + "enemy_hit.png");
	texEnemyDie = renderSys->loadTexture(prefix + "enemy_die.png");

		//audioShoot = loadAudio(prefix + "audio/shoot.wav");
		//audioShootHit = loadAudio(prefix + "audio/wall_hit.wav");
		//audioEnemyHit = loadAudio(prefix + "audio/shoot_hit.wav");
		//musicMain = loadAudio(prefix + "audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");

	// load the map XML and preload image(s)
	map = tmx::loadMap(prefix + "maps/smallmap.tmx");
	for (tmx::TileSet &tileSet : map->tileSets)
	{
		TileSetTextures tst;
		tst.firstGid = tileSet.firstgid;
		tst.textures.reserve(tileSet.tiles.size());

		for (tmx::Tile &tile : tileSet.tiles)
		{
			const std::string imagePath = prefix + "tiles/" + std::filesystem::path(tile.image.source).filename().string();
			//tst.textures.push_back(loadTexture(imagePath));
		}
		//tilesetTextures.push_back(std::move(tst));
	}

	Node &root = world.getNode(hRoot);

	// add the background elements
	NodeHandle hBgLayer = world.createNode();
	Node &bgLayer = world.getNode(hBgLayer);

	NodeHandle hBG1 = world.createNode();
	Node &bg1 = world.getNode(hBG1);
	services.compSys().addComponent<SpriteComponent>(bg1, texBg1, static_cast<float>(state.logW), static_cast<float>(state.logH))
		.setFollowViewport(false);
	bgLayer.addChild(bg1);

	NodeHandle hBG4 = world.createNode();
	Node &bg4 = world.getNode(hBG4);
	auto &bg4Sc = services.compSys().addComponent<SpriteComponent>(bg4, texBg4, static_cast<float>(state.logW), static_cast<float>(state.logH));
	bg4Sc.setFollowViewport(false);
	bg4Sc.setParalaxFactor(0.05f);
	bgLayer.addChild(bg4);

	NodeHandle hBG3 = world.createNode();
	Node &bg3 = world.getNode(hBG3);
	auto &bg3Sc = services.compSys().addComponent<SpriteComponent>(bg3, texBg3, static_cast<float>(state.logW), static_cast<float>(state.logH));
	bg3Sc.setFollowViewport(false);
	bg3Sc.setParalaxFactor(0.1f);
	bgLayer.addChild(bg3);

	NodeHandle hBG2 = world.createNode();
	Node &bg2 = world.getNode(hBG2);
	auto &bg2Sc = services.compSys().addComponent<SpriteComponent>(bg2, texBg2, static_cast<float>(state.logW), static_cast<float>(state.logH));
	bg2Sc.setFollowViewport(false);
	bg2Sc.setParalaxFactor(0.2f);
	bgLayer.addChild(bg2);

	root.addChild(bgLayer);

	// load the map layers
	for (auto &layer : map->layers)
	{
		switch (layer.index())
		{
			case 0:
			{
				processLayer(root, services, std::get<tmx::Layer>(layer));
			}
			case 1:
			{
				processLayer(root, services, std::get<tmx::ObjectGroup>(layer));
			}
		}
	}

	// start up gameplay systems
	services.compSys().registerSystem(std::make_unique<PlayerControlSystem>(services));
	services.compSys().registerSystem(std::make_unique<WeaponSystem>(services));
	services.compSys().registerSystem(std::make_unique<ProjectileSystem>(services));
	services.compSys().registerSystem(std::make_unique<BasicCameraSystem>(services, glm::vec2(state.logW, state.logH), map->tileWidth, map->tileHeight, map->mapWidth, map->mapHeight));
	services.compSys().registerSystem(std::make_unique<DamageSystem>(services));
	services.compSys().registerSystem(std::make_unique<EnemySystem>(services));

	return true;
}

auto Platformer::createObject(Services &services, int r, int c)
{
	World &world = services.world();
	NodeHandle newObjHandle = world.createNode();
	Node &obj = world.getNode(newObjHandle);

	obj.setPosition(glm::vec2(
		c * map->tileWidth,
		r * map->tileHeight));
	return newObjHandle;
}
void Platformer::processLayer(Node &root, Services &services, tmx::Layer &layer) // Tile layers
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
				auto &renderComponent = services.compSys().addComponent<SpriteComponent>(tile, texEnemy, map->tileWidth, map->tileHeight);
				renderComponent.setTexture(tex);
				// only level tiles get a collision component
				if (layer.name == "Level")
				{
					auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(tile);
					collisionComponent.setCollider(SDL_FRect{
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
void Platformer::processLayer(Node &root, Services &services, tmx::ObjectGroup &objectGroup) // Object layers
{
	World &world = services.world();
	NodeHandle hLayer = world.createNode();
	Node &layerObject = world.getNode(hLayer);

	for (tmx::LayerObject &obj : objectGroup.objects)
	{
		glm::vec2 objPos(
			obj.x - map->tileWidth / 2,
			obj.y - map->tileHeight / 2);

		if (obj.type == "Player")
		{
			NodeHandle hPlayer = world.createNode();
			Node &player = world.getNode(hPlayer);
			player.setTag(2);
			player.setPosition(objPos);
			auto &inputComponent = services.compSys().addComponent<InputComponent>(player, hPlayer);
			auto &ctrlComp = services.compSys().addComponent<PlayerControllerComponent>(player);
			ctrlComp.setIdleAnimation(ANIM_PLAYER_IDLE);
			ctrlComp.setIdleTexture(texIdle);
			ctrlComp.setRunAnimation(ANIM_PLAYER_RUN);
			ctrlComp.setRunTexture(texRun);
			ctrlComp.setJumpAnimation(ANIM_PLAYER_JUMP);
			ctrlComp.setJumpTexture(texRun);
			ctrlComp.setSlideAnimation(ANIM_PLAYER_SLIDE);
			ctrlComp.setSlideTexture(texSlide);
			ctrlComp.setSlideShootAnimation(ANIM_PLAYER_SLIDE_SHOOT);
			ctrlComp.setSlideShootTexture(texSlideShoot);
			ctrlComp.setShootAnimation(ANIM_PLAYER_SHOOT);
			ctrlComp.setShootTexture(texShoot);
			ctrlComp.setRunShootAnimation(ANIM_PLAYER_RUN);
			ctrlComp.setRunShootTexture(texRunShoot);

			// setup the player's weapon
			WeaponComponent &wpnComp = services.compSys().addComponent<WeaponComponent>(player);
			wpnComp.animProjectile = animBulletMoving;
			wpnComp.animProjectileHit = animBulletHit;
			wpnComp.texProjectile = texBullet;
			wpnComp.texProjectileHit = texBulletHit;

			auto &physicsComponent = services.compSys().addComponent<PhysicsComponent>(player);
			physicsComponent.setAcceleration(glm::vec2(800, 0));
			physicsComponent.setMaxSpeed(glm::vec2(100, 300));
			//physicsComponent.setDynamic(true);
			auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(player);
			collisionComponent.setCollider(SDL_FRect{
				.x = 11, .y = 6,
				.w = 10, .h = 26
				});
			auto &animComponent = services.compSys().addComponent<AnimationComponent>(player, playerAnims);
			auto &renderComponent = services.compSys().addComponent<SpriteComponent>(player, texIdle, map->tileWidth, map->tileHeight);
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
			//physicsComponent.setDynamic(true);
			auto &collisionComponent = services.compSys().addComponent<CollisionComponent>(enemy);
			collisionComponent.setCollider(SDL_FRect{
				.x = 10, .y = 4, .w = 12, .h = 28
				});
			services.compSys().addComponent<HealthComponent>(enemy, 300);
			auto &animComponent = services.compSys().addComponent<AnimationComponent>(enemy, enemyAnims);
			animComponent.setAnimation(ANIM_ENEMY);
			auto &renderComponent = services.compSys().addComponent<SpriteComponent>(enemy, texEnemy, map->tileWidth, map->tileHeight);
			services.compSys().addComponent<EnemyComponent>(enemy, EnemyType::creeper);

			layerObject.addChild(enemy);
		}
	}
	root.addChild(layerObject);
}

void Platformer::cleanup()
{
}
