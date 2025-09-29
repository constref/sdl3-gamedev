#include "physicscomponent.h"
#include "inputcomponent.h"
#include "../framecontext.h"
#include "../gameobject.h"
#include "../events.h"
#include "../commands.h"

PhysicsComponent::PhysicsComponent(GameObject &owner, InputComponent *inputComponent) : Component(owner)
{
	direction = 0;
	maxSpeedX = 0;
	velocity = acceleration = glm::vec2(0);
	grounded = false;

	if (inputComponent)
	{
		// request direction updates from input component
		inputComponent->directionUpdate.addObserver([this](float direction) {
			this->direction = direction;
		});
	}
}

void PhysicsComponent::onAttached()
{
	owner.getCommandDispatch().registerCommand(Commands::AddImpulse, this);
	owner.getCommandDispatch().registerCommand(Commands::SetGrounded, this);
	owner.getCommandDispatch().registerCommand(Commands::IntegrateVelocityX, this);
	owner.getCommandDispatch().registerCommand(Commands::IntegrateVelocityY, this);
	owner.getCommandDispatch().registerCommand(Commands::ZeroVelocityX, this);
	owner.getCommandDispatch().registerCommand(Commands::ZeroVelocityY, this);
}

void PhysicsComponent::update(const FrameContext &ctx)
{
	// apply some gravity
	glm::vec2 vel = getVelocity();
	vel+= glm::vec2(0, 500) * ctx.deltaTime;

	// horizontal movement
	if (direction)
	{
		vel+= direction * acceleration * ctx.deltaTime;
		if (std::abs(vel.x) > maxSpeedX)
		{
			vel.x = direction * maxSpeedX;
		}

		// check if sliding
		if (vel.x * direction < 0)
		{
			emit(ctx, static_cast<int>(Events::slide));
		}
	}
	else
	{
		// decelerate
		const float factor = vel.x > 0 ? -1.5f : 1.5f;
		float amount = factor * acceleration.x * ctx.deltaTime;
		if (std::abs(vel.x) < std::abs(amount))
		{
			vel.x = 0;
			emit(ctx, static_cast<int>(Events::idle));
		}
		else
		{
			vel.x += amount;
		}
	}
	setVelocity(vel);
}

void PhysicsComponent::onCommand(const Command &command)
{
	if (command.id == Commands::AddImpulse)
	{
		if (isGrounded())
		{
			setGrounded(false);
			const glm::vec2 *JUMP_IMPULSE = static_cast<const glm::vec2 *>(command.param.asPtr);
			setVelocity(getVelocity() + *JUMP_IMPULSE);
		}
	}
	else if (command.id == Commands::SetGrounded)
	{
		setGrounded(command.param.asBool);
	}
	else if (command.id == Commands::IntegrateVelocityX)
	{
		owner.setPosition(owner.getPosition() + glm::vec2(getVelocity().x * command.param.asFloat, 0));
	}
	else if (command.id == Commands::IntegrateVelocityY)
	{
		owner.setPosition(owner.getPosition() + glm::vec2(0, getVelocity().y * command.param.asFloat));
	}
	else if (command.id == Commands::ZeroVelocityX)
	{
		glm::vec2 vel = getVelocity();
		vel.x = 0;
		setVelocity(vel);
	}
	else if (command.id == Commands::ZeroVelocityY)
	{
		glm::vec2 vel = getVelocity();
		vel.y = 0;
		setVelocity(vel);
	}
}

