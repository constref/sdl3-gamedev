#include <SDL3/SDL.h>
#include <algorithm>
#include <format>
#include "collisioncomponent.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../gamestate.h"
#include "../events.h"
#include "../commands.h"

#include "physicscomponent.h"

std::vector<CollisionComponent *> CollisionComponent::allComponents;

CollisionComponent::CollisionComponent(GameObject &owner, PhysicsComponent *physicsComp) : Component(owner), collider{ 0 }
{
	dynamic = false;
	// keep track of all collision components
	allComponents.push_back(this);

	velocity = glm::vec2(0);
	if (physicsComp)
	{
		physicsComp->velocityUpdate.addObserver([this](glm::vec2 velocity) {
			this->velocity = velocity;
			});
	}
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
						//genericResponse(ctx, owner, otherOwner, overlap, foundGround);

						if (axis == 1 && overlap.x)
						{
							if (velocity.x > 0) // from left
							{
								owner.setPosition(owner.getPosition() - glm::vec2(overlap.x, 0));
							}
							else if (velocity.x < 0) // from right
							{
								owner.setPosition(owner.getPosition() + glm::vec2(overlap.x, 0));
							}
							owner.getCommandDispatch().submit(Command { .id = Commands::ZeroVelocityX });

							//if (owner.getController())
							//{
							//	owner.getController()->collisionHandler(otherOwner, overlap);
							//}
						}
						else if (axis == 2 && overlap.y)
						{
							if (velocity.y > 0) // from top
							{
								owner.setPosition(owner.getPosition() - glm::vec2(0, overlap.y));
								emit(ctx, static_cast<int>(Events::landed));
							}
							else if (velocity.y < 0) // from bottom
							{
								owner.setPosition(owner.getPosition() + glm::vec2(0, overlap.y));
							}
							owner.getCommandDispatch().submit(Command { .id = Commands::ZeroVelocityY });

							//if (owner.getController())
							//{
							//	owner.getController()->collisionHandler(otherOwner, overlap);
							//}
						}
					}
				}
			}
		}
		};

	// integrate X velocity first and check for collisions
	Command cmdVelX;
	cmdVelX.id = Commands::IntegrateVelocityX;
	cmdVelX.param.asFloat = ctx.deltaTime;
	owner.getCommandDispatch().submit(cmdVelX);

	checkCollisions(1);

	Command cmdVelY;
	cmdVelY.id = Commands::IntegrateVelocityY;
	cmdVelY.param.asFloat = ctx.deltaTime;
	owner.getCommandDispatch().submit(cmdVelY);

	checkCollisions(2);

	if (velocity.y > 0)
	{
		emit(ctx, static_cast<int>(Events::falling));
	}
}

void CollisionComponent::genericResponse(const FrameContext &ctx, GameObject &objA, GameObject &objB, glm::vec2 &overlap, bool &foundGround)
{
	//printf(std::format("{}, {}\n", overlap.x, overlap.y).c_str());
	// colliding on the x-axis
	PhysicsComponent *physCmp = owner.getComponent<PhysicsComponent>();
	if (physCmp)
	{
		if (velocity.y > 0 && overlap.y <= overlap.x)
		{
			// collision below
			objA.setPosition(objA.getPosition() - glm::vec2(0, overlap.y));
			physCmp->setVelocity(glm::vec2(physCmp->getVelocity().x, 0));
			emit(ctx, static_cast<int>(Events::landed));
		}
		else if (velocity.y < 0 && overlap.y < overlap.x)
		{
			objA.setPosition(objA.getPosition() + glm::vec2(0, overlap.y));
			physCmp->setVelocity(glm::vec2(physCmp->getVelocity().x, 0));
		}
		else
		{
			if (velocity.x > 0)
			{
				objA.setPosition(objA.getPosition() - glm::vec2(overlap.x, 0));
			}
			else if (velocity.x < 0)
			{
				objA.setPosition(objA.getPosition() + glm::vec2(overlap.x, 0));
			}
			physCmp->setVelocity(glm::vec2(0, physCmp->getVelocity().y));
		}
	}
	/*
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
	*/
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
