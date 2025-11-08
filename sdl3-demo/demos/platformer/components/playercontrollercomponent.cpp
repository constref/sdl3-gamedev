#include "playercontrollercomponent.h"

#include <framecontext.h>
#include <messaging/messaging.h>
#include <node.h>
#include <logger.h>

PlayerControllerComponent::PlayerControllerComponent(Node &owner)
	: Component(owner, ComponentStage::Gameplay), slideTimer(0.16f)
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
	shootAnimationIndex = 0;
	shootTexture = nullptr;

	owner.getCommandDispatcher().registerHandler<UpdateVelocityCommand>(this);
	owner.getCommandDispatcher().registerHandler<UpdateDirectionCommand>(this);

	owner.getEventDispatcher().registerHandler<CollisionEvent>(this);
	owner.getEventDispatcher().registerHandler<FallingEvent>(this);
	owner.getEventDispatcher().registerHandler<JumpEvent>(this);
	owner.getEventDispatcher().registerHandler<ShootBeginEvent>(this);
	owner.getEventDispatcher().registerHandler<ShootEndEvent>(this);
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
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, idleAnimationIndex, idleTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::shooting:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, shootAnimationIndex, shootTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::running:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, runAnimationIndex, runTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::runningShooting:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, runShootAnimationIndex, runShootTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::sliding:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, slideAnimationIndex, slideTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::slidingShooting:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, slideShootAnimationIndex, slideShootTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::airborne:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, runAnimationIndex, runTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::airborneShooting:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, runShootAnimationIndex, runShootTexture, AnimationPlaybackMode::continuous);
			break;
		}
		case PState::falling:
		{
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, runAnimationIndex, runTexture, AnimationPlaybackMode::continuous);
			break;
		}
	}
	currentState = newState;
}

void PlayerControllerComponent::update()
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
				slideTimer.reset();
			}
			break;
		}
		case PState::sliding:
		case PState::slidingShooting:
		{
			if (slideTimer.step(FrameContext::global().deltaTime))
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
				(!shooting ? PState::running : PState::runningShooting) :
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
	if (currentState != PState::airborne && currentState != PState::airborneShooting)
	{
		transitionState(!shooting ? PState::airborne : PState::airborneShooting);
		glm::vec2 jumpImpulse(0, -250.0f);
		owner.sendCommand(AddImpulseCommand{ jumpImpulse });
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
