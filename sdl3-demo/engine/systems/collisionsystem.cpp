#include <systems/collisionsystem.h>
#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>

enum class Axis : int
{
	X = 0,
	Y = 1,
};

CollisionSystem::CollisionSystem(Services &services) : System(services)
{
}

void CollisionSystem::update(Node &node)
{
	auto [pc, cc] = getRequiredComponents(node);

	//if (pc->isDynamic())
	{
		auto &prevContacts = cc->getPrevContacts();
		std::array<bool, 4> contacts = { false }; // left, right, top, bottom

		const auto checkCollisions = [this, &node, &contacts, &prevContacts, pc, cc](glm::vec2 &position, Axis axis) {
			for (NodeHandle handle : CollisionComponent::collidableNodes)
			{
				SDL_FRect rectA{
					.x = position.x + cc->getCollider().x,
					.y = position.y + cc->getCollider().y,
					.w = cc->getCollider().w,
					.h = cc->getCollider().h
				};

				auto &otherOwner = services.world().getNode(handle);
				CollisionComponent *comp = otherOwner.getComponent<CollisionComponent>();
				if (comp != cc)
				{
					SDL_FRect rectB{
						.x = otherOwner.getPosition().x + comp->getCollider().x,
						.y = otherOwner.getPosition().y + comp->getCollider().y,
						.w = comp->getCollider().w,
						.h = comp->getCollider().h
					};

					glm::vec2 overlap{ 0 };
					if (intersectAABB(rectA, rectB, overlap))
					{
						// found intersection, respond accordingly
						if (axis == Axis::X && overlap.x)
						{
							if (pc->getVelocity().x > 0) // from left
							{
								position.x -= overlap.x;
								contacts[0] = true;
								if (!prevContacts[0])
								{
									services.eventQueue().enqueue<CollisionEvent>(node.getHandle(), 0,
										otherOwner.getHandle(), overlap, glm::vec2(-1, 0));
								}
							}
							else if (pc->getVelocity().x < 0) // from right
							{
								position.x += overlap.x;
								contacts[1] = true;
								if (!prevContacts[1])
								{
									services.eventQueue().enqueue<CollisionEvent>(node.getHandle(), 0,
										otherOwner.getHandle(), overlap, glm::vec2(1, 0));
								}
							}
							pc->setVelocity(pc->getVelocity() * glm::vec2(0, 1));
						}
						else if (axis == Axis::Y && overlap.y)
						{
							if (pc->getVelocity().y > 0) // from top
							{
								position.y -= overlap.y;
								contacts[2] = true;
								if (!prevContacts[2])
								{
									services.eventQueue().enqueue<CollisionEvent>(node.getHandle(), 0,
										otherOwner.getHandle(), overlap, glm::vec2(0, 1));
								}
							}
							else if (pc->getVelocity().y < 0) // from bottom
							{
								position.y += overlap.y;
								contacts[3] = true;
								if (!prevContacts[3])
								{
									services.eventQueue().enqueue<CollisionEvent>(node.getHandle(), 0,
										otherOwner.getHandle(), overlap, glm::vec2(0, -1));
								}
							}
							pc->setVelocity(pc->getVelocity() * glm::vec2(1, 0));
						}
					}
				}
			}
		};

		glm::vec2 tentativePos = node.getPosition();
		tentativePos.x += pc->getDelta().x;
		checkCollisions(tentativePos, Axis::X);
		tentativePos.y += pc->getDelta().y;
		checkCollisions(tentativePos, Axis::Y);

		node.setPosition(tentativePos);

		if (prevContacts[2] && !contacts[2])
		{
			services.eventQueue().enqueue<FallingEvent>(node.getHandle(), 0);
		}

		for (int i = 0; i < contacts.size(); ++i)
		{
			prevContacts[i] = contacts[i];
		}
	}
}

bool CollisionSystem::intersectAABB(const SDL_FRect &a, const SDL_FRect &b, glm::vec2 &overlap)
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
