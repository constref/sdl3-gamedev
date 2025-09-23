#include <SDL3/SDL.h>
#include <algorithm>
#include "collisioncomponent.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../gamestate.h"
#include "../events.h"

std::vector<CollisionComponent *> CollisionComponent::allComponents;

CollisionComponent::CollisionComponent(std::shared_ptr<GameObject> owner) : Component(owner), collider{0}
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
	auto o = owner.lock();
	SDL_FRect rectA{
		.x = o->position.x + collider.x,
		.y = o->position.y + collider.y,
		.w = collider.w,
		.h = collider.h
	};

	for (auto comp : allComponents)
	{
		if (comp != this)
		{
			auto otherOwner = comp->owner.lock();
			SDL_FRect rectB{
				.x = otherOwner->position.x + comp->collider.x,
				.y = otherOwner->position.y + comp->collider.y,
				.w = comp->collider.w,
				.h = comp->collider.h
			};
			if (collider.w != 0 && collider.h != 0)
			{
				glm::vec2 overlap{ 0 };
				if (intersectAABB(rectA, rectB, overlap))
				{
					// found intersection, respond accordingly
					genericResponse(*o, *otherOwner, overlap);
					if (onCollision)
					{
						onCollision(otherOwner, overlap);
					}
				}
			}
		}
	}
}

void CollisionComponent::genericResponse(GameObject &objA, GameObject &objB, glm::vec2 &overlap)
{
	// colliding on the x-axis
	if (overlap.x < overlap.y)
	{
		if (objA.position.x < objB.position.x) // from left
		{
			objA.position.x -= overlap.x;
		}
		else // from right
		{
			objA.position.x += overlap.x;
		}
	}
	else
	{
		if (objA.position.y < objB.position.y) // from top
		{
			objA.position.y -= overlap.y;
			emit(static_cast<int>(Events::landed));
		}
		else // from bottom
		{
			objA.position.y += overlap.y;
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
		(minYA <= maxYB && maxYA >= minYB))
	{
		overlap.x = std::min(maxXA - minXB, maxXB - minXA);
		overlap.y = std::min(maxYA - minYB, maxYB - minYA);
		return true;
	}
	return false;
}

void CollisionComponent::setOnCollision(CollisionCallback callback)
{
	this->onCollision = callback;
}
