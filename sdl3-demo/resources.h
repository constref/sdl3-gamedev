#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "sdlstate.h"
#include "animation.h"
#include "tmx.h"

struct TileSetTextures
{
	int firstGid;
	std::vector<SDL_Texture *> textures;
};

struct Resources
{
	static Resources &getInstance()
	{
		static Resources instance;
		return instance;
	}

	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUN = 1;
	const int ANIM_PLAYER_SLIDE = 2;
	const int ANIM_PLAYER_SHOOT = 3;
	const int ANIM_PLAYER_SLIDE_SHOOT = 4;
	std::vector<Animation> playerAnims;
	const int ANIM_BULLET_MOVING = 0;
	const int ANIM_BULLET_HIT = 1;
	std::vector<Animation> bulletAnims;
	const int ANIM_ENEMY = 0;
	const int ANIM_ENEMY_HIT = 1;
	const int ANIM_ENEMY_DIE = 2;
	std::vector<Animation> enemyAnims;

	std::vector<SDL_Texture *> textures;
	SDL_Texture *texIdle, *texRun, *texBrick, *texGrass, *texGround, *texPanel,
		*texSlide, *texBg1, *texBg2, *texBg3, *texBg4, *texBullet, *texBulletHit,
		*texShoot, *texRunShoot, *texSlideShoot, *texEnemy, *texEnemyHit, *texEnemyDie;

	//std::vector<MIX_Audio *> audioBuffers;
	//MIX_Audio *audioShoot, *audioShootHit, *audioEnemyHit;
	//MIX_Audio *musicMain;

	std::vector<TileSetTextures> tilesetTextures;
	std::unique_ptr<tmx::Map> map;

	SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filepath);

	//MIX_Audio *loadAudio(const std::string &filepath)
	//{
	//	MIX_Audio* audio = MIX_LoadAudio(nullptr, filepath.c_str(), true);
	//	audioBuffers.push_back(audio);
	//	//MIX_VolumeChunk(chunk, MIX_MAX_VOLUME / 2);
	//	return audio;
	//}

	void load(SDL_Renderer *renderer)
	{
		playerAnims.resize(5);
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
		bulletAnims.resize(2);
		bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.05f);
		bulletAnims[ANIM_BULLET_HIT] = Animation(4, 0.15f);
		enemyAnims.resize(3);
		enemyAnims[ANIM_ENEMY] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_HIT] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

		texIdle = loadTexture(renderer, "data/idle.png");
		texRun = loadTexture(renderer, "data/run.png");
		texSlide = loadTexture(renderer, "data/slide.png");
		texBrick = loadTexture(renderer, "data/tiles/brick.png");
		texGrass = loadTexture(renderer, "data/tiles/grass.png");
		texGround = loadTexture(renderer, "data/tiles/ground.png");
		texPanel = loadTexture(renderer, "data/tiles/panel.png");
		texBg1 = loadTexture(renderer, "data/bg/bg_layer1.png");
		texBg2 = loadTexture(renderer, "data/bg/bg_layer2.png");
		texBg3 = loadTexture(renderer, "data/bg/bg_layer3.png");
		texBg4 = loadTexture(renderer, "data/bg/bg_layer4.png");
		texBullet = loadTexture(renderer, "data/bullet.png");
		texBulletHit = loadTexture(renderer, "data/bullet_hit.png");
		texShoot = loadTexture(renderer, "data/shoot.png");
		texRunShoot = loadTexture(renderer, "data/shoot_run.png");
		texSlideShoot = loadTexture(renderer, "data/slide_shoot.png");
		texEnemy = loadTexture(renderer, "data/enemy.png");
		texEnemyHit = loadTexture(renderer, "data/enemy_hit.png");
		texEnemyDie = loadTexture(renderer, "data/enemy_die.png");

		//audioShoot = loadAudio("data/audio/shoot.wav");
		//audioShootHit = loadAudio("data/audio/wall_hit.wav");
		//audioEnemyHit = loadAudio("data/audio/shoot_hit.wav");
		//musicMain = loadAudio("data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");

		// load the map XML and preload image(s)
		map = tmx::loadMap("data/maps/smallmap.tmx");
		for (tmx::TileSet &tileSet : map->tileSets)
		{
			TileSetTextures tst;
			tst.firstGid = tileSet.firstgid;
			tst.textures.reserve(tileSet.tiles.size());

			for (tmx::Tile &tile : tileSet.tiles)
			{
				const std::string imagePath = "data/tiles/" + std::filesystem::path(tile.image.source).filename().string();
				tst.textures.push_back(loadTexture(renderer, imagePath));
			}
			tilesetTextures.push_back(std::move(tst));
		}
	}

	void unload()
	{
		for (SDL_Texture *tex : textures)
		{
			SDL_DestroyTexture(tex);
		}

		//for (MIX_Audio *audio : audioBuffers)
		//{
		//	MIX_DestroyAudio(audio);
		//}
	}
};

