#include "physicscomponent.h"
#include "inputcomponent.h"
#include "../framecontext.h"
#include "../gameobject.h"
#include "../events.h"
#include "../commands.h"

PhysicsComponent::PhysicsComponent(GameObject &owner, InputComponent *inputComponent) : Component(owner)
{
	direction = 1;
	maxSpeedX = 0;
	velocity = acceleration = glm::vec2(0);
	grounded = false;

	if (inputComponent)
	{
		inputComponent->directionUpdate.addObserver([this](float direction) {
			this->direction = direction;
		});
	}
}

void PhysicsComponent::update(const FrameContext &ctx)
{
	// apply some gravity
	glm::vec2 vel = getVelocity();
	vel+= glm::vec2(0, 500) * ctx.deltaTime;

	// horizontal movement
	printf("dir: %f\n", direction);
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
	owner.setPosition(owner.getPosition() + vel * ctx.deltaTime);
}

void PhysicsComponent::onAttached()
{
	owner.getCommandDispatch().registerCommand(Commands::Jump, this);
	owner.getCommandDispatch().registerCommand(Commands::SetGrounded, this);
}

void PhysicsComponent::onCommand(const Command &command)
{
	if (command.id == Commands::Jump)
	{
		const glm::vec2 JUMP_FORCE(0, -200.0f);
		if (isGrounded())
		{
			setGrounded(false);
			setVelocity(getVelocity() + JUMP_FORCE);
		}
	}
	else if (command.id == Commands::SetGrounded)
	{
		setGrounded(command.param.asBool);
	}
}

