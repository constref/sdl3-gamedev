#pragma once

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <vector>
#include "animation.h"

enum class PlayerState
{
	idle, running, jumping
};

enum class BulletState
{
	flying, disintagrating, inactive
};

enum class EnemyState
{
	shambling, damaged, dead
};

enum class ObjectType
{
	player, enemy, level, bullet, foreground, background
};

static const float PISTOL_TIME = 0.3f;
static const float ASSAULT_RIFLE_TIME = 0.1f;

struct PlayerData
{
	PlayerState state;
	Timer weaponTimer;
	bool isShooting;

	PlayerData() : weaponTimer(PISTOL_TIME)
	{
		state = PlayerState::idle;
		isShooting = false;
	}
};

struct BulletData
{
	BulletState state;
	BulletData() : state(BulletState::flying) {}
};

struct EnemyData
{
	EnemyState state;
	int hp;
	Timer dmgTimer;
	Timer thinkTimer;

	EnemyData() :
		state(EnemyState::shambling), hp(10), dmgTimer(0.5f), thinkTimer(1.0f) {}
};

struct LevelData {};

union ObjectData
{
	PlayerData player;
	BulletData bullet;
	EnemyData enemy;
	LevelData level;

	ObjectData() : level(LevelData()) {}
};


struct GameObject
{
	ObjectType type;
	bool dynamic;
	glm::vec2 position, velocity, acceleration;
	SDL_FRect collider;
	SDL_Texture *texture;
	bool isGrounded;
	std::vector<Animation> animations;
	unsigned int currentAnimation;
	ObjectData data;
	float direction;
	bool shouldFlash;
	Timer flashTimer;

	GameObject() : flashTimer(0.05f)
	{
		type = ObjectType::level;
		dynamic = false;
		data.level = LevelData();
		position = velocity = acceleration = glm::vec2(0, 0);
		collider = SDL_FRect{
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0
		};
		texture = nullptr;
		isGrounded = false;
		currentAnimation = 0;
		direction = 1;
		shouldFlash = false;
	}
};
