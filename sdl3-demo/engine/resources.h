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
	static Resources &get()
	{
		static Resources instance;
		return instance;
	}

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
		const std::string prefix = "data/";

		playerAnims.resize(6);
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_JUMP] = Animation(4, 1.0f);
		playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
		bulletAnims.resize(2);
		bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.01f);
		bulletAnims[ANIM_BULLET_HIT] = Animation(4, 0.15f);
		enemyAnims.resize(3);
		enemyAnims[ANIM_ENEMY] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_HIT] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

		texIdle = loadTexture(renderer, prefix + "idle.png");
		texRun = loadTexture(renderer, prefix + "run.png");
		texSlide = loadTexture(renderer, prefix + "slide.png");
		texBrick = loadTexture(renderer, prefix + "tiles/brick.png");
		texGrass = loadTexture(renderer, prefix + "tiles/grass.png");
		texGround = loadTexture(renderer, prefix + "tiles/ground.png");
		texPanel = loadTexture(renderer, prefix + "tiles/panel.png");
		texBg1 = loadTexture(renderer, prefix + "bg/bg_layer1.png");
		texBg2 = loadTexture(renderer, prefix + "bg/bg_layer2.png");
		texBg3 = loadTexture(renderer, prefix + "bg/bg_layer3.png");
		texBg4 = loadTexture(renderer, prefix + "bg/bg_layer4.png");
		texBullet = loadTexture(renderer, prefix + "bullet.png");
		texBulletHit = loadTexture(renderer, prefix + "bullet_hit.png");
		texShoot = loadTexture(renderer, prefix + "shoot.png");
		texRunShoot = loadTexture(renderer, prefix + "shoot_run.png");
		texSlideShoot = loadTexture(renderer, prefix + "slide_shoot.png");
		texEnemy = loadTexture(renderer, prefix + "enemy.png");
		texEnemyHit = loadTexture(renderer, prefix + "enemy_hit.png");
		texEnemyDie = loadTexture(renderer, prefix + "enemy_die.png");

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

