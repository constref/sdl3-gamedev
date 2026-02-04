#pragma once

#include <systems/system.h>
#include <components/collisioncomponent.h>
#include <components/physicscomponent.h>
#include "../components/enemycomponent.h"

class DeathEvent;

class EnemySystem : public System<FrameStage::Gameplay, EnemyComponent, PhysicsComponent, CollisionComponent>
{
	int animEnemyDeath = -1;
public:
	EnemySystem(Services &services, int animEnemyDeath);
	void update(Node &node) {}

	void onEvent(NodeHandle target, const DeathEvent &event);
};