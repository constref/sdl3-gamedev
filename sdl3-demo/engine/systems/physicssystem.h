#pragma once

#include "system.h"
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

class PhysicsSystem : public System<FrameStage::Physics, PhysicsComponent, CollisionComponent>
{
public:
	void update(Node &node) override;
};