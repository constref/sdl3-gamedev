#include "physicscomponent.h"

#include "../framecontext.h"
#include "../gameobject.h"
#include "../messaging/events.h"
#include "../messaging/messages.h"
#include "../messaging/coresubjects.h"

PhysicsComponent::PhysicsComponent(GameObject &owner) : Component(owner)
{
	direction = 0;
	maxSpeed = glm::vec2(0, 0);
	velocity = acceleration = glm::vec2(0);
	grounded = false;
	netForce = glm::vec2(0);
}

void PhysicsComponent::onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<PhysicsComponent, IntegrateVelocityMessage>(this);
	msgDispatch.registerHandler<PhysicsComponent, ScaleVelocityAxisMessage>(this);
	msgDispatch.registerHandler<PhysicsComponent, AddImpulseMessage>(this);

	registry.registerSubject(CoreSubjects::VELOCITY, &velocitySubject);
}

void PhysicsComponent::registerObservers(SubjectRegistry &registry)
{
	registry.addObserver<float>(CoreSubjects::DIRECTION, [this](const float &direction) {
		this->direction = direction;
	});
}

void PhysicsComponent::update(const FrameContext &ctx)
{
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
}

void PhysicsComponent::onMessage(const IntegrateVelocityMessage &msg)
{
	glm::vec2 pos = owner.getPosition();
	pos[static_cast<int>(msg.getAxis())] += velocity[static_cast<int>(msg.getAxis())] * msg.getDeltaTime();
	owner.setPosition(pos);
}

void PhysicsComponent::onMessage(const ScaleVelocityAxisMessage &msg)
{
	glm::vec2 vel = getVelocity();
	vel[static_cast<int>(msg.getAxis())] *= msg.getFactor();
	setVelocity(vel);
}

void PhysicsComponent::onMessage(const AddImpulseMessage &msg)
{
	glm::vec2 vel = getVelocity();
	vel += msg.getImpulse();
	setVelocity(vel);
}

//void PhysicsComponent::onCommand(const Command &command)
//{
	//else if (command.id == Commands::AddForce)
	//{
	//	const glm::vec2 *force = static_cast<const glm::vec2 *>(command.param.asPtr);
	//	netForce += *force;
	//}
	//else if (command.id == Commands::SetGrounded)
	//{
	//	setGrounded(command.param.asBool);
	//}
//}

