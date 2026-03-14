#include <systems/physicssystem.h>
#include <world.h>
#include <framecontext.h>
#include <node.h>
#include <logger.h>
#include <format>
#include <messaging/eventqueue.h>
#include <messaging/events.h>

PhysicsSystem::PhysicsSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<DirectionChangedEvent>(this);
}

void PhysicsSystem::update(Node &node)
{
	auto [pc, cc] = getRequiredComponents(node);
	glm::vec3 vel = pc->getVelocity();

	// calculate movement
	glm::vec3 netForce{ 0 };
	netForce += pc->getDirection().x * pc->localX() * pc->getAcceleration().x;
	netForce += pc->getDirection().y * pc->localY() * pc->getAcceleration().y;
	netForce += pc->getDirection().z * pc->localZ() * pc->getAcceleration().z;

	// gravity
	const glm::vec3 gravity(0, 600, 0);
	netForce += gravity * pc->getGravityFactor();

	// apply forces
	vel += netForce * FrameContext::dt();

	const glm::vec3 maxSpeed = pc->getMaxSpeed();
	for (int i = 0; i < 3; i++)
	{
		const float absVal = std::abs(vel[i]);
		if (absVal > maxSpeed[i])
		{
			const float dir = vel[i] / absVal;
			vel[i] = dir * maxSpeed[i];
		}
	}

	const float absVelY = std::abs(vel.y);
	if (absVelY > maxSpeed.y)
	{
		const float yDir = vel.y / absVelY;
		vel.y = yDir * maxSpeed.y;
	}

	// simulate friction
	const float factor = std::max(0.9f, 1.0f - pc->getDamping() * FrameContext::dt());
	vel.x *= factor;
	if (std::abs(vel.x) < 0.01f)
	{
		vel.x = 0;
	}
	vel.z *= factor;
	if (std::abs(vel.z) < 0.01f)
	{
		vel.z = 0;
	}
	pc->setVelocity(vel);

	// collision component can check per-axis and apply resolution
	pc->setDelta(vel * FrameContext::dt());
}

void PhysicsSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event) const
{
	Node &node = services.world().getNode(target);
	auto *pc = node.getComponent<PhysicsComponent>();
	pc->setDirection(event.getDirection());
}
