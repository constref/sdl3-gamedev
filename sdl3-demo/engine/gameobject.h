#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "animation.h"
#include "components/component.h"
#include "messaging/messagedispatch.h"

#include <string>
#include <unordered_map>
#include <messaging/observer.h>

enum class BulletState
{
	moving, colliding, inactive
};

enum class EnemyState
{
	shambling, damaged, dead
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

class GameObject
{
	glm::vec2 position;
	std::vector<std::shared_ptr<GameObject>> children;
	std::vector<Component *> components;
	MessageDispatch msgDispatch;
	SubjectRegistry subjectRegistry;
	bool debugHighlight;

	MessageDispatch &getMessageDispatch() { return msgDispatch; }

public:
	GameObject()
	{
		position = glm::vec2(0);
		debugHighlight = false;
	}

	~GameObject()
	{
		for (auto *comp : components)
		{
			delete comp;
		}
		components.clear();
	}

	glm::vec2 getPosition() const { return position; }

	void setPosition(const glm::vec2 position) { this->position = position; }
	bool isDebugHighlight() const { return debugHighlight; }
	void setDebugHighlight(bool highlight) { debugHighlight = highlight; }

	template<typename T, typename... Args>
	T &addComponent(Args... args)
	{
		T *comp = new T(*this, args...);
		components.push_back(comp);
		return *comp;
	}

	void initializeComponents()
	{
		// register subjects
		for (Component *cmp : components)
		{
			cmp->onAttached(subjectRegistry, getMessageDispatch());
		}

		// register observers
		for (Component *cmp : components)
		{
			cmp->registerObservers(subjectRegistry);
		}
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

	template<typename MessageType>
	void sendMessage(const MessageType &message)
	{
		msgDispatch.send(message);
	}

	void notify(const FrameContext &ctx, int eventId)
	{
		for (auto &comp : components)
		{
			comp->onEvent(eventId);
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