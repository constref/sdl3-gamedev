#pragma once

#include <vector>
#include <systems/system.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

struct SDL_FRect;

class CollisionSystem : public System<FrameStage::Physics, PhysicsComponent, CollisionComponent>
{
public:
	CollisionSystem(Services &services);
	void update(Node &node) override;

	bool intersectAABB(const Collider &a, const Collider &b, glm::vec3 &overlap);
};