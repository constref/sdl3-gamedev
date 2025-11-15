#include "collisioncomponent.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <format>

#include <node.h>
#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <logger.h>

std::vector<NodeHandle> CollisionComponent::collidableNodes;

CollisionComponent::CollisionComponent(Node &owner) : Component(owner, FrameStage::Physics), collider{ 0 }
{
	collidableNodes.push_back(owner.getHandle());
	hasCollider = true;

	velocity = glm::vec2(0);
	prevContacts[0] = false;
	prevContacts[1] = false;
	prevContacts[2] = false;
	prevContacts[3] = false;
}

CollisionComponent::~CollisionComponent()
{
	if (hasCollider)
	{
		removeCollider();
	}
}

void CollisionComponent::removeCollider()
{
	auto itr = std::find(collidableNodes.begin(), collidableNodes.end(), owner.getHandle());
	if (itr != collidableNodes.end())
	{
		collidableNodes.erase(itr);
		hasCollider = false;
	}
	else
	{
		Logger::error(this, "Collision component not found.");
	}
}
