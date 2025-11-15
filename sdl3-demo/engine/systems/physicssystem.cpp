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
	if (pc->isDynamic())
	{
		glm::vec2 vel = pc->getVelocity();

		glm::vec2 netForce{ 0 };
		netForce += pc->getDirection() * pc->getAcceleration();

		// gravity
		const glm::vec2 gravity(0, 600);
		netForce += gravity * pc->getGravityFactor();

		// apply forces
		vel += netForce * FrameContext::dt();

		const float absVelX = std::abs(vel.x);
		glm::vec2 maxSpeed = pc->getMaxSpeed();
		if (absVelX > maxSpeed.x)
		{
			const float xDir = vel.x / absVelX;
			vel.x = xDir * maxSpeed.x;
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
		pc->setVelocity(vel);

		// collision component can check per-axis and apply resolution
		pc->setDelta(vel * FrameContext::dt());
	}
}

void PhysicsSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event)
{
	Node &node = services.world().getNode(target);
	auto *pc = node.getComponent<PhysicsComponent>();
	pc->setDirection(event.getDirection());
}
