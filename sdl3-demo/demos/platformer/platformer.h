#pragma once

#include <nodehandle.h>
#include <animation.h>
#include <tmx.h>
#include <memory>
// TODO: Don't want to access render sys directly for asset loading
#include <systems/vulkanrendersystem.h>

struct SDLState;
struct FrameContext;
class Services;
class Node;

struct TileSetTextures
{
	int firstGid;
	std::vector<ResourceId> textures;
};


class Platformer
{
	NodeHandle hRoot;
	NodeHandle hPlayer;

	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUN = 1;
	const int ANIM_PLAYER_JUMP = 2;
	const int ANIM_PLAYER_SLIDE = 3;
	const int ANIM_PLAYER_SHOOT = 4;
	const int ANIM_PLAYER_SLIDE_SHOOT = 5;
	std::vector<Animation> playerAnims;
	const int ANIM_BULLET_MOVING = 0;
	const int ANIM_BULLET_HIT = 1;
	std::vector<Animation> bulletAnims;
	const int ANIM_ENEMY = 0;
	const int ANIM_ENEMY_HIT = 1;
	const int ANIM_ENEMY_DIE = 2;
	std::vector<Animation> enemyAnims;

	ResourceId animBulletMoving, animBulletHit;

	ResourceId texIdle, texRun, texBrick, texGrass,
		texGround, texPanel, texSlide, texBg1, texBg2,
		texBg3, texBg4, texBullet, texBulletHit, texShoot,
		texRunShoot, texSlideShoot, texEnemy, texEnemyHit, texEnemyDie;

	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	std::unique_ptr<tmx::Map> map;
	std::vector<TileSetTextures> tilesetTextures;
	auto createObject(Services &services, int r, int c);
	void processLayer(Node &root, Services &services, tmx::Layer &layer);
	void processLayer(Node &root, Services &services, tmx::ObjectGroup &objectGroup);

public:
	Platformer();

	NodeHandle getPlayerHandle() const { return hPlayer; }
	bool initialize(Services &services, SDLState &state);
	void cleanup();
	void onStart();
	void update();

	auto getRoot() const
	{
		return hRoot;
	}
};
