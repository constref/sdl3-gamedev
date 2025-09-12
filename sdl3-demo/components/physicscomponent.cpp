#include "physicscomponent.h"
#include "inputcomponent.h"
#include "../framecontext.h"
#include "../gameobject.h"

PhysicsComponent::PhysicsComponent(InputComponent *inputComponent) : Component()
{
	direction = 1;
	maxSpeedX = 0;
	velocity = acceleration = glm::vec2(0);
	dynamic = false;
	grounded = false;

	if (inputComponent)
	{
		inputComponent->directionUpdate.addObserver([this](float direction) {
			this->direction = direction;
			});
	}
}

void PhysicsComponent::update(GameObject &owner, const FrameContext &ctx)
{
	if (direction)
	{
		velocity += direction * acceleration * ctx.deltaTime;
		if (std::abs(velocity.x) > maxSpeedX)
		{
			velocity.x = direction * maxSpeedX;
		}
	}
	else
	{
		// decelerate
		if (velocity.x)
		{
			const float factor = velocity.x > 0 ? -1.5f : 1.5f;
			float amount = factor * acceleration.x * ctx.deltaTime;
			if (std::abs(velocity.x) < std::abs(amount))
			{
				velocity.x = 0;
			}
			else
			{
				velocity.x += amount;
			}
		}

	}
	owner.position += velocity * ctx.deltaTime;
}

