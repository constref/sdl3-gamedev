#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include <array>
#include "component.h"

struct SDL_FRect;
class UpdateVelocityCommand;
class TentativeVelocityCommand;
class NodeRemovalEvent;

class CollisionComponent : public Component
{
	// TODO: Will remove static vector in favor of a spatial partitioning structure
	static std::vector<CollisionComponent *> allComponents;
	SDL_FRect collider;
	glm::vec2 velocity;
	std::array<bool, 4> prevContacts; // left, right, top, bottom

public:
	CollisionComponent(Node &owner);
	~CollisionComponent();
	void update(const FrameContext &ctx) override;
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;
	void onCommand(const UpdateVelocityCommand &cmd);
	void onCommand(const TentativeVelocityCommand &cmd);
	void onEvent(const NodeRemovalEvent &event);

	bool intersectAABB(const SDL_FRect &a, const SDL_FRect &b, glm::vec2 &overlap);
	void setCollider(const SDL_FRect &collider) { this->collider = collider; }
};