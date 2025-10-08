#include <SDL3/SDL.h>
#include <algorithm>
#include <format>

#include "collisioncomponent.h"

#include "../gameobject.h"
#include "../framecontext.h"
#include "../gamestate.h"
#include "../messaging/events.h"
#include "../messaging/messages.h"
#include "../messaging/coresubjects.h"

std::vector<CollisionComponent *> CollisionComponent::allComponents;

CollisionComponent::CollisionComponent(GameObject &owner) : Component(owner), collider{ 0 }
{
	dynamic = false;
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

void CollisionComponent::update(const FrameContext &ctx)
{
	const auto checkCollisions = [this, &ctx](int axis) {
		bool foundGround = false;
		for (auto comp : allComponents)
		{
			if (isDynamic())
			{
				SDL_FRect rectA{
					.x = owner.getPosition().x + collider.x,
					.y = owner.getPosition().y + collider.y,
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
						if (ctx.gs.debugMode)
						{
							otherOwner.setDebugHighlight(true);
						}

						// found intersection, respond accordingly
						if (axis == 0 && overlap.x)
						{
							if (velocity.x > 0) // from left
							{
								owner.setPosition(owner.getPosition() - glm::vec2(overlap.x, 0));
								owner.sendMessage(CollisionMessage { otherOwner, overlap, glm::vec2(-1, 0) });
							}
							else if (velocity.x < 0) // from right
							{
								owner.setPosition(owner.getPosition() + glm::vec2(overlap.x, 0));
								owner.sendMessage(CollisionMessage { otherOwner, overlap, glm::vec2(1, 0) });
							}
							owner.sendMessage(ScaleVelocityAxisMessage{ Axis::X, 0.0f });
						}
						else if (axis == 1 && overlap.y)
						{
							if (velocity.y > 0) // from top
							{
								owner.setPosition(owner.getPosition() - glm::vec2(0, overlap.y));
								owner.sendMessage(CollisionMessage { otherOwner, overlap, glm::vec2(0, 1) });
								emit(ctx, static_cast<int>(Events::landed));
							}
							else if (velocity.y < 0) // from bottom
							{
								owner.setPosition(owner.getPosition() + glm::vec2(0, overlap.y));
								owner.sendMessage(CollisionMessage { otherOwner, overlap, glm::vec2(0, -1) });
							}
							owner.sendMessage(ScaleVelocityAxisMessage{ Axis::Y, 0.0f });
						}
					}
				}
			}
		}
	};

	// integrate X velocity first and check for collisions
	owner.sendMessage(IntegrateVelocityMessage{ Axis::X, ctx.deltaTime });
	checkCollisions(0);
	// integrate X velocity first and check for collisions
	owner.sendMessage(IntegrateVelocityMessage{ Axis::Y, ctx.deltaTime });
	checkCollisions(1);

	if (velocity.y > 0)
	{
		emit(ctx, static_cast<int>(Events::falling));
	}
}

void CollisionComponent::registerObservers(SubjectRegistry &registry)
{
	registry.addObserver<glm::vec2>(CoreSubjects::VELOCITY, [this](const glm::vec2 &velocity) {
		this->velocity = velocity;
	});
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
