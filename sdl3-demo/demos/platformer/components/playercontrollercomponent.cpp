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
	shooting = false;
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
	dataDispatcher.registerHandler<UpdateVelocityCommand>(this);
	dataDispatcher.registerHandler<UpdateDirectionCommand>(this);

	eventDispatcher.registerHandler<CollisionEvent>(this);
	eventDispatcher.registerHandler<FallingEvent>(this);
	eventDispatcher.registerHandler<JumpEvent>(this);
	eventDispatcher.registerHandler<ShootBeginEvent>(this);
	eventDispatcher.registerHandler<ShootEndEvent>(this);
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
		case PState::shooting:
		{
			owner.pushData(SetAnimationCommand{ shootAnimationIndex, shootTexture });
			break;
		}
		case PState::running:
		{
			owner.pushData(SetAnimationCommand{ runAnimationIndex, runTexture });
			break;
		}
		case PState::runningShooting:
		{
			owner.pushData(SetAnimationCommand{ runShootAnimationIndex, runShootTexture });
			break;
		}
		case PState::sliding:
		{
			owner.pushData(SetAnimationCommand{ slideAnimationIndex, slideTexture });
			break;
		}
		case PState::slidingShooting:
		{
			owner.pushData(SetAnimationCommand{ slideShootAnimationIndex, slideShootTexture });
			break;
		}
		case PState::airborne:
		{
			owner.pushData(SetAnimationCommand{ runAnimationIndex, runTexture });
			break;
		}
		case PState::airborneShooting:
		{
			owner.pushData(SetAnimationCommand{ runShootAnimationIndex, runShootTexture });
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
		case PState::shooting:
		{
			// holding a direction, start running
			if (direction)
			{
				transitionState(!shooting ? PState::running : PState::runningShooting);
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
		case PState::runningShooting:
		{
			if (direction == 0)
			{
				// no longer holding direction, go to idle
				transitionState(!shooting ? PState::idle : PState::shooting);
			}
			else if (direction * velocity.x < 0)
			{
				// if direction we're holding is opposite to velocity, use sliding state
				transitionState(!shooting ? PState::sliding : PState::slidingShooting);
			}
			break;
		}
		case PState::sliding:
		case PState::slidingShooting:
		{
			if (direction == 0)
			{
				// if no longer holding direction, go to idle
				transitionState(!shooting ? PState::idle : PState::shooting);
			}
			else if (direction * velocity.x > 0)
			{
				// if direction and velocity directions match, go to running
				transitionState(!shooting ? PState::running : PState::runningShooting);
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
		if (currentState == PState::airborne || currentState == PState::airborneShooting)
		{
			transitionState(velocity.x != 0 ?
				(!shooting ? PState::running : PState::runningShooting)  :
				(!shooting ? PState::idle : PState::shooting));
		}
	}
}

void PlayerControllerComponent::onEvent(const FallingEvent &event)
{
	if (currentState != PState::airborne)
	{
		transitionState(!shooting ? PState::airborne : PState::airborneShooting);
	}
}

void PlayerControllerComponent::onEvent(const JumpEvent &event)
{
	if (currentState != PState::airborne)
	{
		transitionState(!shooting ? PState::airborne : PState::airborneShooting);
		glm::vec2 jumpImpulse(0, -250.0f);
		owner.pushData(AddImpulseCommand{ jumpImpulse });
	}
}

void PlayerControllerComponent::onEvent(const ShootBeginEvent &event)
{
	shooting = true;
	switch (currentState)
	{
		case PState::idle:
		{
			transitionState(PState::shooting);
			break;
		}
		case PState::running:
		{
			transitionState(PState::runningShooting);
			break;
		}
		case PState::airborne:
		{
			transitionState(PState::airborneShooting);
			break;
		}
		case PState::sliding:
		{
			transitionState(PState::slidingShooting);
			break;
		}
	}
}
void PlayerControllerComponent::onEvent(const ShootEndEvent &event)
{
	shooting = false;
	switch (currentState)
	{
		case PState::shooting:
		{
			transitionState(PState::idle);
			break;
		}
		case PState::runningShooting:
		{
			transitionState(PState::running);
			break;
		}
		case PState::airborneShooting:
		{
			transitionState(PState::airborne);
			break;
		}
		case PState::slidingShooting:
		{
			transitionState(PState::slidingShooting);
		}
	}
}
