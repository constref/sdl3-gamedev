#include "playercontrollercomponent.h"
#include <framecontext.h>
#include "../messaging/events.h"
#include "../messaging/messages.h"
#include "../../gameobject.h"

PlayerControllerComponent::PlayerControllerComponent(GameObject &owner) : Component(owner)
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

void PlayerControllerComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<PlayerControllerComponent, JumpMessage>(this);
	msgDispatch.registerHandler<PlayerControllerComponent, CollisionMessage>(this);
	msgDispatch.registerHandler<PlayerControllerComponent, FallingMessage>(this);
	msgDispatch.registerHandler<PlayerControllerComponent, VelocityMessage>(this);
	msgDispatch.registerHandler<PlayerControllerComponent, DirectionMessage>(this);
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
			owner.sendMessage(SetAnimationMessage{ idleAnimationIndex, idleTexture });
			break;
		}
		case PState::running:
		{
			owner.sendMessage(SetAnimationMessage{ runAnimationIndex, runTexture });
			break;
		}
		case PState::sliding:
		{
			owner.sendMessage(SetAnimationMessage{ slideAnimationIndex, slideTexture });
			break;
		}
		case PState::airborne:
		{
			owner.sendMessage(SetAnimationMessage{ runAnimationIndex, runTexture });
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
					owner.sendMessage(ScaleVelocityAxisMessage{ Axis::X, factor });
					if (std::abs(velocity.x) < 0.01f)
					{
						owner.sendMessage(ScaleVelocityAxisMessage{ Axis::X, 0.0f });
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

void PlayerControllerComponent::onMessage(const JumpMessage &msg)
{
	if (currentState != PState::airborne)
	{
		transitionState(PState::airborne);
		glm::vec2 jumpImpulse(0, -250.0f);
		owner.sendMessage(AddImpulseMessage{ jumpImpulse });
	}
}

void PlayerControllerComponent::onMessage(const CollisionMessage &msg)
{
	if (glm::dot(msg.getNormal(), glm::vec2(0, 1)) == 1.0f) // landed on something
	{
		if (currentState == PState::airborne)
		{
			transitionState(velocity.x != 0 ? PState::running : PState::idle);
		}
	}
}

void PlayerControllerComponent::onMessage(const FallingMessage &msg)
{
	if (currentState != PState::airborne)
	{
		transitionState(PState::airborne);
	}
}

void PlayerControllerComponent::onMessage(const VelocityMessage &msg)
{
	this->velocity = msg.getVelocity();
}

void PlayerControllerComponent::onMessage(const DirectionMessage &msg)
{
	this->direction = msg.getDirection();
}
