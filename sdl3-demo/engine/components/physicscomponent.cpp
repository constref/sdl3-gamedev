#include "physicscomponent.h"

#include "../framecontext.h"
#include "../gameobject.h"
#include "../messaging/events.h"
#include "../messaging/messages.h"
#include "../messaging/coresubjects.h"

PhysicsComponent::PhysicsComponent(GameObject &owner) : Component(owner)
{
	direction = 0;
	maxSpeedX = 0;
	velocity = acceleration = glm::vec2(0);
	grounded = false;
	netForce = glm::vec2(0);
	mass = 0;
}

void PhysicsComponent::onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch)
{
	//owner.getCommandDispatch().registerCommand(Commands::AddImpulse, this);
	//owner.getCommandDispatch().registerCommand(Commands::SetGrounded, this);
	//owner.getCommandDispatch().registerCommand(Commands::IntegrateVelocityX, this);
	//owner.getCommandDispatch().registerCommand(Commands::IntegrateVelocityY, this);
	//owner.getCommandDispatch().registerCommand(Commands::ZeroVelocityX, this);
	//owner.getCommandDispatch().registerCommand(Commands::ZeroVelocityY, this);

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

	netForce += direction * acceleration * mass;

	// gravity
	const glm::vec2 gravity(0, 9.81f);
	netForce += gravity * mass;

	// apply forces
	vel += netForce * ctx.deltaTime;


	//if (std::abs(vel.x) > maxSpeedX)
	//{
	//	vel.x = direction * maxSpeedX;
	//}

	//if (!direction)
	//{
	//	// decelerate
	//	const float factor = vel.x > 0 ? -1.5f : 1.5f;
	//	float amount = factor * acceleration.x * ctx.deltaTime;
	//	if (std::abs(vel.x) < std::abs(amount) && vel.x != 0)
	//	{
	//		vel.x = 0;
	//		emit(ctx, static_cast<int>(Events::idle));
	//	}
	//	else
	//	{
	//		vel.x += amount;
	//	}
	//}
	setVelocity(vel);
	netForce = glm::vec2(0);
}

void PhysicsComponent::onMessage(const IntegrateVelocityMessage &msg)
{
	owner.getPosition()[static_cast<int>(msg.getAxis())] += velocity[static_cast<int>(msg.getAxis())] * msg.getDeltaTime();
}

//void PhysicsComponent::onCommand(const Command &command)
//{
	//if (command.id == Commands::AddImpulse)
	//{
	//	const glm::vec2 *impulse = static_cast<const glm::vec2 *>(command.param.asPtr);
	//}
	//else if (command.id == Commands::AddForce)
	//{
	//	const glm::vec2 *force = static_cast<const glm::vec2 *>(command.param.asPtr);
	//	netForce += *force;
	//}
	//else if (command.id == Commands::SetGrounded)
	//{
	//	setGrounded(command.param.asBool);
	//}
	//else if (command.id == Commands::IntegrateVelocityX)
	//{
	//	owner.setPosition(owner.getPosition() + glm::vec2(getVelocity().x * command.param.asFloat, 0));
	//}
	//else if (command.id == Commands::IntegrateVelocityY)
	//{
	//	owner.setPosition(owner.getPosition() + glm::vec2(0, getVelocity().y * command.param.asFloat));
	//}
	//else if (command.id == Commands::ZeroVelocityX)
	//{
	//	glm::vec2 vel = getVelocity();
	//	vel.x = 0;
	//	setVelocity(vel);
	//}
	//else if (command.id == Commands::ZeroVelocityY)
	//{
	//	glm::vec2 vel = getVelocity();
	//	vel.y = 0;
	//	setVelocity(vel);
	//}
//}

