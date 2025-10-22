#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include "component.h"

struct SDL_FRect;
class VelocityMessage;
class TentativeVelocityMessage;

class CollisionComponent : public Component
{
	// TODO: Will remove static vector in favor of a spatial partitioning structure
	static std::vector<CollisionComponent *> allComponents;
	SDL_FRect collider;
	glm::vec2 velocity;

public:
	CollisionComponent(GameObject &owner);
	~CollisionComponent();
	void update(const FrameContext &ctx) override;
	void onAttached(MessageDispatch &msgDispatch) override;
	void onMessage(const VelocityMessage &msg);
	void onMessage(const TentativeVelocityMessage &msg);

	bool intersectAABB(const SDL_FRect &a, const SDL_FRect &b, glm::vec2 &overlap);
	void setCollider(const SDL_FRect &collider) { this->collider = collider; }
};