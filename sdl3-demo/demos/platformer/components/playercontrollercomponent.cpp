#include "playercontrollercomponent.h"

#include <framecontext.h>
#include <messaging/datapumps.h>
#include <gameobject.h>

PlayerControllerComponent::PlayerControllerComponent(GameObject &owner) : Component(owner, ComponentStage::Input)
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

void PlayerControllerComponent::onAttached(DataDispatcher &dataDispatcher)
{
	dataDispatcher.registerHandler<PlayerControllerComponent, JumpDPump>(this);
	dataDispatcher.registerHandler<PlayerControllerComponent, CollisionDPump>(this);
	dataDispatcher.registerHandler<PlayerControllerComponent, FallingDPump>(this);
	dataDispatcher.registerHandler<PlayerControllerComponent, VelocityDPump>(this);
	dataDispatcher.registerHandler<PlayerControllerComponent, DirectionDPump>(this);
}

void PlayerControllerComponent::onStart()
{
	transitionState(PState::idle);
}

void PlayerControllerComponent::transitionState(PState newState)
{
	// on enter actions
	switch (newState)
	{
		case PState::idle:
		{
			owner.sendMessage(SetAnimationDPump{ idleAnimationIndex, idleTexture });
			break;
		}
		case PState::running:
		{
			owner.sendMessage(SetAnimationDPump{ runAnimationIndex, runTexture });
			break;
		}
		case PState::sliding:
		{
			owner.sendMessage(SetAnimationDPump{ slideAnimationIndex, slideTexture });
			break;
		}
		case PState::airborne:
		{
			owner.sendMessage(SetAnimationDPump{ runAnimationIndex, runTexture });
			break;
		}
		case PState::falling:
		{
			owner.sendMessage(SetAnimationDPump{ runAnimationIndex, runTexture });
			break;
		}
	}
	currentState = newState;
}

void PlayerControllerComponent::update(const FrameContext &ctx)
{
	if (velocity.y > 0 && currentState != PState::airborne)
	{
		transitionState(PState::airborne);
	}

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
					owner.sendMessage(ScaleVelocityAxisDPump{ Axis::X, factor });
					if (std::abs(velocity.x) < 0.01f)
					{
						owner.sendMessage(ScaleVelocityAxisDPump{ Axis::X, 0.0f });
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

void PlayerControllerComponent::onData(const JumpDPump &msg)
{
	if (currentState != PState::airborne)
	{
		transitionState(PState::airborne);
		glm::vec2 jumpImpulse(0, -250.0f);
		owner.sendMessage(AddImpulseDPump{ jumpImpulse });
	}
}

void PlayerControllerComponent::onData(const CollisionDPump &msg)
{
	float dotY = glm::dot(msg.getNormal(), glm::vec2(0, 1));
	float dotX = glm::dot(msg.getNormal(), glm::vec2(1, 0));

	if (dotY == 1.0f)
	{
		// landed on something
		if (currentState == PState::airborne)
		{
			transitionState(velocity.x != 0 ? PState::running : PState::idle);
		}
		owner.sendMessage(ScaleVelocityAxisDPump{ Axis::Y, 0.0f });
	}
	else if (dotY == -1.0f)
	{
		// hit something above
		owner.sendMessage(ScaleVelocityAxisDPump{ Axis::Y, 0.0f });
	}
	else if (dotX == 1.0f || dotX == -1.0f)
	{
		owner.sendMessage(ScaleVelocityAxisDPump{ Axis::X, 0.0f });
	}
}

void PlayerControllerComponent::onData(const FallingDPump &msg)
{
	if (currentState != PState::airborne)
	{
		transitionState(PState::airborne);
	}
}

void PlayerControllerComponent::onData(const VelocityDPump &msg)
{
	this->velocity = msg.getVelocity();
}

void PlayerControllerComponent::onData(const DirectionDPump &msg)
{
	this->direction = msg.getDirection();
}
