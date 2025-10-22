#include "collisioncomponent.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <format>

#include "../gameobject.h"
#include "../framecontext.h"
#include "../messaging/events.h"
#include "../messaging/messages.h"

std::vector<CollisionComponent *> CollisionComponent::allComponents;

CollisionComponent::CollisionComponent(GameObject &owner) : Component(owner), collider{ 0 }
{
	// keep track of all collision components
	allComponents.push_back(this);

	velocity = glm::vec2(0);
}

CollisionComponent::~CollisionComponent()
{
	// remove from static list
	auto itr = std::find(allComponents.begin(), allComponents.end(), this);
	if (itr != allComponents.end())
	{
		allComponents.erase(itr);
	}
}

void CollisionComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<CollisionComponent, VelocityMessage>(this);
	msgDispatch.registerHandler<CollisionComponent, TentativeVelocityMessage>(this);
}

void CollisionComponent::update(const FrameContext &ctx)
{
}

void CollisionComponent::onMessage(const VelocityMessage &msg)
{
	this->velocity = msg.getVelocity();
}

void CollisionComponent::onMessage(const TentativeVelocityMessage &msg)
{
	const auto checkCollisions = [this](glm::vec2 &position, Axis axis) {
		for (auto comp : allComponents)
		{
			SDL_FRect rectA{
				.x = position.x + collider.x,
				.y = position.y + collider.y,
				.w = collider.w,
				.h = collider.h
			};

			if (comp != this)
			{
				auto &otherOwner = comp->owner;
				SDL_FRect rectB{
					.x = otherOwner.getPosition().x + comp->collider.x,
					.y = otherOwner.getPosition().y + comp->collider.y,
					.w = comp->collider.w,
					.h = comp->collider.h
				};

				glm::vec2 overlap{ 0 };
				if (intersectAABB(rectA, rectB, overlap))
				{
					// found intersection, respond accordingly
					if (axis == Axis::X && overlap.x)
					{
						if (velocity.x > 0) // from left
						{
							position.x -= overlap.x;
							owner.sendMessage(CollisionMessage{ otherOwner, overlap, glm::vec2(-1, 0) });
						}
						else if (velocity.x < 0) // from right
						{
							position.x += overlap.x;
							owner.sendMessage(CollisionMessage{ otherOwner, overlap, glm::vec2(1, 0) });
						}
					}
					else if (axis == Axis::Y && overlap.y)
					{
						if (velocity.y > 0) // from top
						{
							position.y -= overlap.y;
							owner.sendMessage(CollisionMessage{ otherOwner, overlap, glm::vec2(0, 1) });
						}
						else if (velocity.y < 0) // from bottom
						{
							position.y += overlap.y;
							owner.sendMessage(CollisionMessage{ otherOwner, overlap, glm::vec2(0, -1) });
						}
					}
				}
			}
		}
		};

	glm::vec2 tentativePos = owner.getPosition();
	tentativePos[static_cast<int>(msg.getAxis())] += msg.getDelta();
	checkCollisions(tentativePos, msg.getAxis());
	owner.setPosition(tentativePos);
}

bool CollisionComponent::intersectAABB(const SDL_FRect &a, const SDL_FRect &b, glm::vec2 &overlap)
{
	const float minXA = a.x;
	const float maxXA = a.x + a.w;
	const float minYA = a.y;
	const float maxYA = a.y + a.h;
	const float minXB = b.x;
	const float maxXB = b.x + b.w;
	const float minYB = b.y;
	const float maxYB = b.y + b.h;

	if ((minXA < maxXB && maxXA > minXB) &&
		(minYA < maxYB && maxYA > minYB))
	{
		overlap.x = std::min(maxXA - minXB, maxXB - minXA);
		overlap.y = std::min(maxYA - minYB, maxYB - minYA);
		return true;
	}
	return false;
}
