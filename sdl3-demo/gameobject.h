#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include "animation.h"
#include "components/component.h"

enum class PlayerState
{
	idle, running, jumping
};

enum class BulletState
{
	moving, colliding, inactive
};

enum class EnemyState
{
	shambling, damaged, dead
};

struct PlayerData
{
	PlayerState state;
	Timer weaponTimer;

	PlayerData() : weaponTimer(0.1f)
	{
		state = PlayerState::idle;
	}
};

struct LevelData {};
struct EnemyData
{
	EnemyState state;
	Timer damageTimer;
	int healthPoints;

	EnemyData() : state(EnemyState::shambling), damageTimer(0.5f)
	{
		healthPoints = 100;
	}
};

struct BulletData
{
	BulletState state;
	BulletData() : state(BulletState::moving)
	{
	}
};

union ObjectData
{
	PlayerData player;
	LevelData level;
	EnemyData enemy;
	BulletData bullet;
};

enum class ObjectType
{
	player, level, enemy, bullet
};

struct GameObject
{
	ObjectType type;
	ObjectData data;
	glm::vec2 position, velocity, acceleration;
	float direction;
	float maxSpeedX;
	bool dynamic;
	bool grounded;
	SDL_FRect collider;
	int spriteFrame;

	std::vector<std::shared_ptr<Component>> components;

	GameObject() : data{ .level = LevelData() }, collider{ 0 }
	{
		type = ObjectType::level;
		direction = 1;
		maxSpeedX = 0;
		position = velocity = acceleration = glm::vec2(0);
		dynamic = false;
		grounded = false;
		spriteFrame = 1;
	}

	void update(SDLState &state, GameState &gs, Resources &res, float deltaTime)
	{
		for (auto &comp : components)
		{
			comp->update(state, gs, res, deltaTime);
		}
	}
};