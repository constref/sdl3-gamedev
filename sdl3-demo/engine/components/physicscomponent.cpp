#include "physicscomponent.h"

#include <node.h>
#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/events.h>

PhysicsComponent::PhysicsComponent(Node &owner) : Component(owner, ComponentStage::Physics)
{
	direction = 0;
	maxSpeed = glm::vec2(0, 0);
	velocity = acceleration = glm::vec2(0);
	grounded = false;
	netForce = glm::vec2(0);
	dynamic = false;
	gravityFactor = 1.0f;
	damping = 5.0f;

	owner.getCommandDispatcher().registerHandler<ScaleVelocityAxisCommand>(this);
	owner.getCommandDispatcher().registerHandler<AddImpulseCommand>(this);
	owner.getCommandDispatcher().registerHandler<UpdateDirectionCommand>(this);
}

void PhysicsComponent::setVelocity(const glm::vec2 &vel)
{
	this->velocity = vel;
	owner.sendCommand(UpdateVelocityCommand{ vel });
}

void PhysicsComponent::update(const FrameContext &ctx)
{
	glm::vec2 vel = getVelocity();

	netForce += direction * acceleration;

	// gravity
	const glm::vec2 gravity(0, 600);
	netForce += gravity * gravityFactor;

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

	const glm::vec2 delta = vel * ctx.deltaTime;
	if (isDynamic())
	{
		// collision component can check per-axis and apply resolution
		owner.sendCommand(TentativeVelocityCommand{ delta });
	}

	const float factor = std::max(0.9f, 1.0f - damping * ctx.deltaTime);
	owner.sendCommand(ScaleVelocityAxisCommand{ Axis::X, factor });
	if (std::abs(velocity.x) < 0.01f)
	{
		owner.sendCommand(ScaleVelocityAxisCommand{ Axis::X, 0.0f });
	}
}

void PhysicsComponent::onCommand(const ScaleVelocityAxisCommand &msg)
{
	glm::vec2 vel = getVelocity();
	vel[static_cast<int>(msg.getAxis())] *= msg.getFactor();
	setVelocity(vel);
}

void PhysicsComponent::onCommand(const AddImpulseCommand &msg)
{
	glm::vec2 vel = getVelocity();
	vel += msg.getImpulse();
	setVelocity(vel);
}

void PhysicsComponent::onCommand(const UpdateDirectionCommand &msg)
{
	this->direction = msg.getDirection();
}
