#include "playercontrollercomponent.h"
#include <framecontext.h>
#include "../messaging/observer.h"
#include "../messaging/coresubjects.h"
#include "../messaging/events.h"
#include "../messaging/messages.h"
#include "../../gameobject.h"

PlayerControllerComponent::PlayerControllerComponent(GameObject &owner) : Component(owner)
{
	direction = 0;
	velocity = glm::vec2(0);
	currentState = PState::idle;
	idleAnimationIndex = 0;
	idleTexture = nullptr;
	runAnimationIndex = 0;
	runTexture = nullptr;
	slideAnimationIndex = 0;
	slideTexture = nullptr;
}

void PlayerControllerComponent::onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<PlayerControllerComponent, JumpMessage>(this);
	msgDispatch.registerHandler<PlayerControllerComponent, CollisionMessage>(this);
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
					const float damping = 1.0f;
					const float factor = std::max(0.9f, 1.0f - damping * ctx.deltaTime);
					owner.sendMessage(ScaleVelocityAxisMessage{ Axis::X, factor });
					if (std::abs(velocity.x) < 0.1f)
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

void PlayerControllerComponent::onEvent(int eventId)
{
	glm::vec2 jumpImpulse(0, -200.0f);
	if (currentState == PState::idle)
	{
		if (eventId == static_cast<int>(Events::run))
		{
			transitionState(PState::running);
		}
		else if (eventId == static_cast<int>(Events::jump))
		{
			owner.sendMessage(AddImpulseMessage{ jumpImpulse });
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::falling))
		{
			transitionState(PState::airborne);
		}
	}
	else if (currentState == PState::running)
	{
		if (eventId == static_cast<int>(Events::idle))
		{
			transitionState(PState::idle);
		}
		else if (eventId == static_cast<int>(Events::jump))
		{
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::falling))
		{
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::slide))
		{
			transitionState(PState::sliding);
		}
	}
	else if (currentState == PState::airborne)
	{
		if (eventId == static_cast<int>(Events::landed))
		{
			transitionState(velocity.x != 0 ? PState::running : PState::idle);
			//owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetGrounded, .param = true });
		}
	}
	else if (currentState == PState::sliding)
	{
		if (eventId == static_cast<int>(Events::idle))
		{
			transitionState(PState::idle);
		}
		if (eventId == static_cast<int>(Events::run))
		{
			transitionState(PState::running);
		}
		else if (eventId == static_cast<int>(Events::jump))
		{
			//owner.getCommandDispatch().dispatch(Command{ .id = Commands::AddImpulse, .param {.asPtr = &jumpImpulse } });
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::falling))
		{
			transitionState(PState::airborne);
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
	printf("Collision normal: (%.1f, %.1f)\n", msg.getNormal().x, msg.getNormal().y);
}

void PlayerControllerComponent::registerObservers(SubjectRegistry &registry)
{
	registry.addObserver<float>(CoreSubjects::DIRECTION, [this](const float &direction) {
		this->direction = direction;
	});

	registry.addObserver<glm::vec2>(CoreSubjects::VELOCITY, [this](const glm::vec2 &velocity) {
		this->velocity = velocity;
	});
}
