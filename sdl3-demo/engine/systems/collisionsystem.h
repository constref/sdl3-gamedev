#pragma once

#include "system.h"
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

class CollisionSystem : public System<PhysicsComponent, CollisionComponent>
{
};