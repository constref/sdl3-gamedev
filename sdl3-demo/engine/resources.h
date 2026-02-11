#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "sdlstate.h"
#include "animation.h"
#include "tmx.h"

struct Resources
{
	static Resources &get()
	{
		static Resources instance;
		return instance;
	}

	//std::vector<MIX_Audio *> audioBuffers;
	//MIX_Audio *audioShoot, *audioShootHit, *audioEnemyHit;
	//MIX_Audio *musicMain;

	//MIX_Audio *loadAudio(const std::string &filepath)
	//{
	//	MIX_Audio* audio = MIX_LoadAudio(nullptr, filepath.c_str(), true);
	//	audioBuffers.push_back(audio);
	//	//MIX_VolumeChunk(chunk, MIX_MAX_VOLUME / 2);
	//	return audio;
	//}

	void load()
	{


		//texIdle = loadTexture(prefix + "idle.png");
		//texRun = loadTexture(prefix + "run.png");
		//texSlide = loadTexture(prefix + "slide.png");
		//texBrick = loadTexture(prefix + "tiles/brick.png");
		//texGrass = loadTexture(prefix + "tiles/grass.png");
		//texGround = loadTexture(prefix + "tiles/ground.png");
		//texPanel = loadTexture(prefix + "tiles/panel.png");
		//texBg1 = loadTexture(prefix + "bg/bg_layer1.png");
		//texBg2 = loadTexture(prefix + "bg/bg_layer2.png");
		//texBg3 = loadTexture(prefix + "bg/bg_layer3.png");
		//texBg4 = loadTexture(prefix + "bg/bg_layer4.png");
		//texBullet = loadTexture(prefix + "bullet.png");
		//texBulletHit = loadTexture(prefix + "bullet_hit.png");
		//texShoot = loadTexture(prefix + "shoot.png");
		//texRunShoot = loadTexture(prefix + "shoot_run.png");
		//texSlideShoot = loadTexture(prefix + "slide_shoot.png");
		//texEnemy = loadTexture(prefix + "enemy.png");
		//texEnemyHit = loadTexture(prefix + "enemy_hit.png");
		//texEnemyDie = loadTexture(prefix + "enemy_die.png");

		//audioShoot = loadAudio(prefix + "audio/shoot.wav");
		//audioShootHit = loadAudio(prefix + "audio/wall_hit.wav");
		//audioEnemyHit = loadAudio(prefix + "audio/shoot_hit.wav");
		//musicMain = loadAudio(prefix + "audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");

	}

	void unload()
	{
		//for (MIX_Audio *audio : audioBuffers)
		//{
		//	MIX_DestroyAudio(audio);
		//}
	}
};

