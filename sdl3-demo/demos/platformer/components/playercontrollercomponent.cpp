#include "playercontrollercomponent.h"

#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/events.h>
#include <node.h>

PlayerControllerComponent::PlayerControllerComponent(Node &owner) : Component(owner, ComponentStage::Gameplay)
{
	direction = 0;
	velocity = glm::vec2(0);
	grounded = false;
	currentState = PState::idle;
	idleAnimationIndex = 0;
	idleTexture = nullptr;
	runAnimationIndex = 0;
	runTexture = nullptr;
	slideAnimationIndex = 0;
	slideTexture = nullptr;
}

void PlayerControllerComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	dataDispatcher.registerHandler<PlayerControllerComponent, UpdateVelocityCommand>(this);
	dataDispatcher.registerHandler<PlayerControllerComponent, UpdateDirectionCommand>(this);

	eventDispatcher.registerHandler<PlayerControllerComponent, CollisionEvent>(this);
	eventDispatcher.registerHandler<PlayerControllerComponent, FallingEvent>(this);
	eventDispatcher.registerHandler<PlayerControllerComponent, JumpEvent>(this);
}

void PlayerControllerComponent::onStart()
{
	transitionState(PState::airborne);
}

void PlayerControllerComponent::transitionState(PState newState)
{
	// on enter actions
	switch (newState)
	{
		case PState::idle:
		{
			owner.pushData(SetAnimationCommand{ idleAnimationIndex, idleTexture });
			break;
		}
		case PState::running:
		{
			owner.pushData(SetAnimationCommand{ runAnimationIndex, runTexture });
			break;
		}
		case PState::sliding:
		{
			owner.pushData(SetAnimationCommand{ slideAnimationIndex, slideTexture });
			break;
		}
		case PState::airborne:
		{
			owner.pushData(SetAnimationCommand{ runAnimationIndex, runTexture });
			break;
		}
		case PState::falling:
		{
			owner.pushData(SetAnimationCommand{ runAnimationIndex, runTexture });
			break;
		}
	}
	currentState = newState;
}

void PlayerControllerComponent::update(const FrameContext &ctx)
{
	switch (currentState)
	{
		case PState::idle:
		{
			// holding a direction, start running
			if (direction)
			{
				transitionState(PState::running);
			}
			else
			{
				// direction is zero, decelerate to stop
				if (velocity.x)
				{
					const float damping = 10.0f;
					const float factor = std::max(0.9f, 1.0f - damping * ctx.deltaTime);
					owner.pushData(ScaleVelocityAxisCommand{ Axis::X, factor });
					if (std::abs(velocity.x) < 0.01f)
					{
						owner.pushData(ScaleVelocityAxisCommand{ Axis::X, 0.0f });
					}
				}
			}
			break;
		}
		case PState::running:
		{
			if (direction == 0)
			{
				// no longer holding direction, go to idle
				transitionState(PState::idle);
			}
			else if (direction * velocity.x < 0)
			{
				// if direction we're holding is opposite to velocity, use sliding state
				transitionState(PState::sliding);
			}
			break;
		}
		case PState::sliding:
		{
			if (direction == 0)
			{
				// if no longer holding direction, go to idle
				transitionState(PState::idle);
			}
			else if (direction * velocity.x > 0)
			{
				// if direction and velocity directions match, go to running
				transitionState(PState::running);
			}
			break;
		}
	}
}

void PlayerControllerComponent::onCommand(const UpdateVelocityCommand &msg)
{
	this->velocity = msg.getVelocity();
}

void PlayerControllerComponent::onCommand(const UpdateDirectionCommand &msg)
{
	this->direction = msg.getDirection();
}

void PlayerControllerComponent::onEvent(const CollisionEvent &event)
{
	float dotY = glm::dot(event.getNormal(), glm::vec2(0, 1));
	float dotX = glm::dot(event.getNormal(), glm::vec2(1, 0));

	if (dotY == 1.0f)
	{
		// landed on something
		if (currentState == PState::airborne)
		{
			transitionState(velocity.x != 0 ? PState::running : PState::idle);
		}
	}
}

void PlayerControllerComponent::onEvent(const FallingEvent &event)
{
	if (currentState != PState::airborne)
	{
		transitionState(PState::airborne);
	}
}

void PlayerControllerComponent::onEvent(const JumpEvent &event)
{
	if (currentState != PState::airborne)
	{
		transitionState(PState::airborne);
		glm::vec2 jumpImpulse(0, -250.0f);
		owner.pushData(AddImpulseCommand{ jumpImpulse });
	}
}
