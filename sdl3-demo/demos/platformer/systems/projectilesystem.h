#pragma once

#include <systems/system.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>
#include <components/spritecomponent.h>
#include "../components/projectilecomponent.h"

class CollisionEvent;
class NodeRemovalEvent;

class ProjectileSystem : public System<FrameStage::Gameplay, ProjectileComponent, PhysicsComponent, CollisionComponent, SpriteComponent>
{
public:
	ProjectileSystem(Services &services);
	void update(Node &node);

	void onEvent(NodeHandle target, const CollisionEvent &event);
	void onEvent(NodeHandle target, const NodeRemovalEvent &event);
};