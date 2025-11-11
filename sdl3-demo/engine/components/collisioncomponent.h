#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include <array>

#include <nodehandle.h>
#include "component.h"

struct SDL_FRect;
class UpdateVelocityCommand;
class TentativeVelocityCommand;
class RemoveColliderEvent;

class CollisionComponent : public Component
{
	SDL_FRect collider;
	glm::vec2 velocity;
	std::array<bool, 4> prevContacts; // left, right, top, bottom

public:
	CollisionComponent(Node &owner);
	~CollisionComponent();

	// TODO: Will remove static vector in favor of a spatial partitioning structure
	static std::vector<NodeHandle> collidableNodes;

	void onEvent(const RemoveColliderEvent &event);

	SDL_FRect getCollider() const { return collider; }
	void setCollider(const SDL_FRect &collider) { this->collider = collider; }
	auto &getPrevContacts() { return prevContacts; }
};