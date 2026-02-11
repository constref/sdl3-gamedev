#pragma once

#include <nodehandle.h>
#include <animation.h>
#include <tmx.h>
#include <memory>
#include <applicationmodule.h>
#include <resourceid.h>

struct SDLState;
struct FrameContext;
class Services;
class Node;

struct TileSetTextures
{
	int firstGid;
	std::vector<ResourceId> textures;
};


class Platformer : public ApplicationModule
{
	NodeHandle hRoot;
	NodeHandle hPlayer;

	ResourceId animPlayerIdle, animPlayerRun, animPlayerJump, animPlayerSlide, animPlayerShoot, animPlayerSlideShoot;
	ResourceId animBulletMoving, animBulletHit;
	ResourceId animEnemy, animEnemyHit, animEnemyDie;

	ResourceId texIdle, texRun, texBrick, texGrass,
		texGround, texPanel, texSlide, texBg1, texBg2,
		texBg3, texBg4, texBullet, texBulletHit, texShoot,
		texRunShoot, texSlideShoot, texEnemy, texEnemyHit, texEnemyDie;

	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	std::unique_ptr<tmx::Map> map;
	std::vector<TileSetTextures> tilesetTextures;
	auto createObject(Services &services, int r, int c);
	void processLayer(Node &root, Services &services, SDLState &state, tmx::Layer &layer, int index);
	void processLayer(Node &root, Services &services, SDLState &state, tmx::ObjectGroup &objectGroup, int index);

public:
	Platformer();

	NodeHandle getPlayerHandle() const { return hPlayer; }
	bool initialize(Services &services, SDLState &state) override;
	void start(Services &services, SDLState &state) override;
	void cleanup() override;

	NodeHandle getRoot() const override
	{
		return hRoot;
	}
};
