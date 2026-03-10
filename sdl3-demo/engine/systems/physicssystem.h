#pragma once

#include <systems/system.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

class DirectionChangedEvent;

class PhysicsSystem : public System<FrameStage::Physics, PhysicsComponent, CollisionComponent>
{
	glm::vec3 localX;
	glm::vec3 localY;
	glm::vec3 localZ;
	
public:
	PhysicsSystem(Services &services);

	void update(Node &node) override;
	void onEvent(NodeHandle target, const DirectionChangedEvent &event) const;
};