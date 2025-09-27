#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include "../component.h"

struct SDL_FRect;
class PhysicsComponent;

class CollisionComponent : public Component
{
	// TODO: Will remove static vector in favor of a spatial partitioning structure
	static std::vector<CollisionComponent *> allComponents;
	SDL_FRect collider;
	glm::vec2 velocity;
	bool dynamic;

public:
	CollisionComponent(GameObject &owner, PhysicsComponent *physicsComp = nullptr);
	~CollisionComponent();
	void update(const FrameContext &ctx) override;

	void genericResponse(const FrameContext &ctx, GameObject &objA, GameObject &objB, glm::vec2 &overlap, bool &foundGround);
	bool intersectAABB(const SDL_FRect &a, const SDL_FRect &b, glm::vec2 &overlap);

	bool isDynamic() const { return dynamic; }
	void setDynamic(bool dynamic) { this->dynamic = dynamic; }
	void setCollider(const SDL_FRect &collider) { this->collider = collider; }
};