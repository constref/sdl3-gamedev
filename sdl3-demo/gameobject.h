#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include "animation.h"
#include "component.h"
#include "controller.h"
#include "commanddispatch.h"

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

class GameObject
{
	glm::vec2 position;
	std::vector<Component *> components;
	Controller *controller;
	CommandDispatch commandDispatch;
	bool debugHighlight;

public:
	GameObject()
	{
		controller = nullptr;
		position = glm::vec2(0);
		debugHighlight = false;
	}

	~GameObject()
	{
		if (controller)
		{
			delete controller;
		}
		for (auto *comp : components)
		{
			delete comp;
		}
		components.clear();
	}

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }

	template<typename T, typename... Args>
	void createController(Args... args)
	{
		assert(controller == nullptr);
		this->controller = new T(*this, args...); 
	}
	Controller *getController() const { return controller; }
	CommandDispatch &getCommandDispatch() { return commandDispatch; }

	bool isDebugHighlight() const { return debugHighlight; }
	void setDebugHighlight(bool highlight) { debugHighlight = highlight; }

	template<typename T, typename... Args>
	T &addComponent(Args... args)
	{
		T *comp = new T(*this, args...);
		components.push_back(comp);
		comp->onAttached();
		return *comp;
	}

	template<typename T>
	T *getComponent()
	{
		for (auto *comp : components)
		{
			T *specific = dynamic_cast<T*>(comp);
			if (specific)
			{
				return specific;
			}
		}
		return nullptr;
	}

	void notify(const FrameContext &ctx, int eventId)
	{
		if (controller)
		{
			controller->eventHandler(ctx, eventId);
		}
	}
	
	void update(const FrameContext &ctx)
	{
		for (auto &comp : components)
		{
			comp->update(ctx);
		}
	}
};