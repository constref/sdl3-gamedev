#include "collisioncomponent.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <format>

#include <node.h>
#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>

std::vector<CollisionComponent *> CollisionComponent::allComponents;

CollisionComponent::CollisionComponent(Node &owner) : Component(owner, ComponentStage::Physics), collider{ 0 }
{
	// keep track of all collision components
	allComponents.push_back(this);

	velocity = glm::vec2(0);
	prevContacts[0] = false;
	prevContacts[1] = false;
	prevContacts[2] = false;
	prevContacts[3] = false;
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

void CollisionComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	dataDispatcher.registerHandler<CollisionComponent, UpdateVelocityCommand>(this);
	dataDispatcher.registerHandler<CollisionComponent, TentativeVelocityCommand>(this);
}

void CollisionComponent::update(const FrameContext &ctx)
{
}

void CollisionComponent::onCommand(const UpdateVelocityCommand &dp)
{
	this->velocity = dp.getVelocity();
}

void CollisionComponent::onCommand(const TentativeVelocityCommand &dp)
{
	std::array<bool, 4> contacts = { false }; // left, right, top, bottom
	const auto checkCollisions = [this, &contacts](glm::vec2 &position, Axis axis) {
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
							contacts[0] = true;
							if (!prevContacts[0])
							{
								EventQueue::get().enqueue<CollisionEvent>(owner.getHandle(), ComponentStage::Physics,
									otherOwner.getHandle(), overlap, glm::vec2(-1, 0));
							}
						}
						else if (velocity.x < 0) // from right
						{
							position.x += overlap.x;
							contacts[1] = true;
							if (!prevContacts[1])
							{
								EventQueue::get().enqueue<CollisionEvent>(owner.getHandle(), ComponentStage::Physics,
									otherOwner.getHandle(), overlap, glm::vec2(1, 0));
							}
						}
						owner.pushData(ScaleVelocityAxisCommand{ Axis::X, 0.0f });
					}
					else if (axis == Axis::Y && overlap.y)
					{
						if (velocity.y > 0) // from top
						{
							position.y -= overlap.y;
							contacts[2] = true;
							if (!prevContacts[2])
							{
								EventQueue::get().enqueue<CollisionEvent>(owner.getHandle(), ComponentStage::Physics,
									otherOwner.getHandle(), overlap, glm::vec2(0, 1));
							}
						}
						else if (velocity.y < 0) // from bottom
						{
							position.y += overlap.y;
							contacts[3] = true;
							if (!prevContacts[3])
							{
								EventQueue::get().enqueue<CollisionEvent>(owner.getHandle(), ComponentStage::Physics,
									otherOwner.getHandle(), overlap, glm::vec2(0, -1));
							}
						}
						owner.pushData(ScaleVelocityAxisCommand{ Axis::Y, 0.0f });
					}
				}
			}
		}
	};

	glm::vec2 tentativePos = owner.getPosition();
	tentativePos.x += dp.getDelta().x;
	checkCollisions(tentativePos, Axis::X);
	tentativePos.y += dp.getDelta().y;
	checkCollisions(tentativePos, Axis::Y);

	owner.setPosition(tentativePos);

	if (prevContacts[2] && !contacts[2])
	{
		EventQueue::get().enqueue<FallingEvent>(owner.getHandle(), ComponentStage::Physics);
	}

	for (int i = 0; i < contacts.size(); ++i)
	{
		prevContacts[i] = contacts[i];
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
