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

struct Collider
{
	float x, y, w, h;
};

class CollisionComponent : public Component
{
	Collider collider;
	glm::vec2 velocity;
	std::array<bool, 4> prevContacts; // left, right, top, bottom
	bool hasCollider;

public:
	CollisionComponent(Node &owner);
	~CollisionComponent() override;

	// TODO: Will remove static vector in favor of a spatial partitioning structure
	static std::vector<NodeHandle> collidableNodes;

	const Collider &getCollider() const { return collider; }
	void setCollider(const Collider &collider) { this->collider = collider; }
	auto &getPrevContacts() { return prevContacts; }

	void removeCollider();
};