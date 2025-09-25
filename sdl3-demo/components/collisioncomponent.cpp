#include <SDL3/SDL.h>
#include <algorithm>
#include "collisioncomponent.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../gamestate.h"
#include "../events.h"

std::vector<CollisionComponent *> CollisionComponent::allComponents;

CollisionComponent::CollisionComponent(GameObject &owner) : Component(owner), collider{0}
{
	// keep track of all collision components
	allComponents.push_back(this);
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

void CollisionComponent::update(const FrameContext &ctx)
{
	SDL_FRect rectA{
		.x = owner.getPosition().x + collider.x,
		.y = owner.getPosition().y + collider.y,
		.w = collider.w,
		.h = collider.h
	};

	for (auto comp : allComponents)
	{
		if (comp != this)
		{
			auto &otherOwner = comp->owner;
			SDL_FRect rectB{
				.x = otherOwner.getPosition().x + comp->collider.x,
				.y = otherOwner.getPosition().y + comp->collider.y,
				.w = comp->collider.w,
				.h = comp->collider.h
			};
			if (collider.w != 0 && collider.h != 0)
			{
				glm::vec2 overlap{ 0 };
				if (intersectAABB(rectA, rectB, overlap))
				{
					// found intersection, respond accordingly
					genericResponse(ctx, owner, otherOwner, overlap);
					if (owner.getController())
					{
						owner.getController()->collisionHandler(otherOwner, overlap);
					}
				}
			}
		}
	}
}

void CollisionComponent::genericResponse(const FrameContext &ctx, GameObject &objA, GameObject &objB, glm::vec2 &overlap)
{
	// colliding on the x-axis
	if (overlap.x < overlap.y)
	{
		if (objA.getPosition().x < objB.getPosition().x) // from left
		{
			objA.setPosition(objA.getPosition() - glm::vec2(overlap.x, 0));
		}
		else // from right
		{
			objA.setPosition(objA.getPosition() + glm::vec2(overlap.x, 0));
		}
	}
	else
	{
		if (objA.getPosition().y < objB.getPosition().y) // from top
		{
			objA.setPosition(objA.getPosition() - glm::vec2(0, overlap.y));
			emit(ctx, static_cast<int>(Events::landed));
		}
		else // from bottom
		{
			objA.setPosition(objA.getPosition() + glm::vec2(0, overlap.y));
		}
	}
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
