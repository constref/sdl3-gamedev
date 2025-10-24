#include "physicscomponent.h"

#include <gameobject.h>
#include <framecontext.h>
#include <messaging/datapumps.h>

// TODO: Want to remove this CollisionComponent include
#include <components/collisioncomponent.h>

PhysicsComponent::PhysicsComponent(GameObject &owner) : Component(owner, ComponentStage::Physics)
{
	direction = 0;
	maxSpeed = glm::vec2(0, 0);
	velocity = acceleration = glm::vec2(0);
	grounded = false;
	netForce = glm::vec2(0);
	dynamic = false;
	hasCollider = false;
}

void PhysicsComponent::onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	dataDispatcher.registerHandler<PhysicsComponent, ScaleVelocityAxisDPump>(this);
	dataDispatcher.registerHandler<PhysicsComponent, AddImpulseDPump>(this);
	dataDispatcher.registerHandler<PhysicsComponent, DirectionDPump>(this);
}

void PhysicsComponent::setVelocity(const glm::vec2 &vel)
{
	this->velocity = vel;
	owner.sendMessage(VelocityDPump{ vel });
}

void PhysicsComponent::update(const FrameContext &ctx)
{
	if (owner.getComponent<CollisionComponent>())
	{
		hasCollider = true;
	}

	glm::vec2 vel = getVelocity();

	netForce += direction * acceleration;

	// gravity
	const glm::vec2 gravity(0, 600);
	netForce += gravity;

	// apply forces
	vel += netForce * ctx.deltaTime;

	const float absVelX = std::abs(vel.x);
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

	setVelocity(vel);
	netForce = glm::vec2(0);

	if (isDynamic())
	{
		const glm::vec2 tentative = vel * ctx.deltaTime;
		if (hasCollider)
		{
			// collision component can check per-axis and apply resolution
			owner.sendMessage(TentativeVelocityDPump{ tentative.x, Axis::X });
			owner.sendMessage(TentativeVelocityDPump{ tentative.y, Axis::Y });
		}
		else
		{
			owner.setPosition(owner.getPosition() + tentative);
		}
	}
}

void PhysicsComponent::onData(const ScaleVelocityAxisDPump &msg)
{
	glm::vec2 vel = getVelocity();
	vel[static_cast<int>(msg.getAxis())] *= msg.getFactor();
	setVelocity(vel);
}

void PhysicsComponent::onData(const AddImpulseDPump &msg)
{
	glm::vec2 vel = getVelocity();
	vel += msg.getImpulse();
	setVelocity(vel);
}

void PhysicsComponent::onData(const DirectionDPump &msg)
{
	this->direction = msg.getDirection();
}
