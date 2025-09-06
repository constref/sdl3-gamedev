#include "physicscomponent.h"
#include "inputcomponent.h"
#include "../framecontext.h"
#include "../gameobject.h"

PhysicsComponent::PhysicsComponent(InputComponent *inputComponent, GameObject &owner) : Component(owner)
{
	direction = 1;
	if (inputComponent)
	{
		inputComponent->directionUpdate.addObserver([this](float direction) {
			this->direction = direction;
		});
	}
}

void PhysicsComponent::update(const FrameContext &ctx)
{
	if (direction)
	{
		owner.velocity += direction * owner.acceleration * ctx.deltaTime;
		if (std::abs(owner.velocity.x) > owner.maxSpeedX)
		{
			owner.velocity.x = direction * owner.maxSpeedX;
		}
	}
	else
	{
		// decelerate
		if (owner.velocity.x)
		{
			const float factor = owner.velocity.x > 0 ? -1.5f : 1.5f;
			float amount = factor * owner.acceleration.x * ctx.deltaTime;
			if (std::abs(owner.velocity.x) < std::abs(amount))
			{
				owner.velocity.x = 0;
			}
			else
			{
				owner.velocity.x += amount;
			}
		}

	}
	owner.position += owner.velocity * ctx.deltaTime;
}

