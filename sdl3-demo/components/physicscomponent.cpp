#include "physicscomponent.h"
#include "inputcomponent.h"
#include "../framecontext.h"
#include "../gameobject.h"

PhysicsComponent::PhysicsComponent(std::shared_ptr<GameObject> owner, InputComponent *inputComponent) : Component(owner)
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

void PhysicsComponent::update(const FrameContext &ctx)
{
	// apply some gravity
	if (dynamic && !grounded)
	{
		velocity += glm::vec2(0, 500) * ctx.deltaTime;
	}

	// horizontal movement
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
	if (auto o = owner.lock())
	{
		o->position += velocity * ctx.deltaTime;
	}
}

