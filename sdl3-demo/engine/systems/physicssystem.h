#pragma once

#include "system.h"
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

class DirectionChangedEvent;

class PhysicsSystem : public System<FrameStage::Physics, PhysicsComponent, CollisionComponent>
{
public:
	PhysicsSystem();

	void update(Node &node) override;
	void onEvent(NodeHandle target, const DirectionChangedEvent &event);
};