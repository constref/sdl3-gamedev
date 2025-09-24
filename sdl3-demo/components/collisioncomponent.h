#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include "component.h"

struct SDL_FRect;

class CollisionComponent : public Component
{
	// TODO: Will remove static vector in favor of a spatial partitioning structure
	static std::vector<CollisionComponent *> allComponents;

	using CollisionCallback = std::function<void(GameObject &other, glm::vec2 overlap)>;

	CollisionCallback onCollision;
	SDL_FRect collider;

public:
	CollisionComponent(GameObject &owner);
	~CollisionComponent();
	virtual void update(const FrameContext &ctx) override;

	void genericResponse(GameObject &objA, GameObject &objB, glm::vec2 &overlap);
	bool intersectAABB(const SDL_FRect &a, const SDL_FRect &b, glm::vec2 &overlap);
	void setCollider(const SDL_FRect &collider) { this->collider = collider; }
	void setOnCollision(CollisionCallback callback);
};