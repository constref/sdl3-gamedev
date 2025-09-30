#include "statecomponent.h"

#include "../resources.h"
#include "../gameobject.h"
#include "../commands.h"
#include "../events.h"

StateComponent::StateComponent(GameObject &owner) : Component(owner)
{
	currentState = PState::idle;
}

void StateComponent::transitionState(PState newState)
{
	Resources &res = Resources::getInstance();

	// on enter actions
	switch (newState)
	{
		case PState::idle:
		{
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetAnimation, .param = idleAnimationIndex });
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetTexture, .param {.asPtr = idleTexture } });
			break;
		}
		case PState::running:
		{
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetAnimation, .param = runAnimationIndex });
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetTexture, .param {.asPtr = runTexture } });
			break;
		}
		case PState::sliding:
		{
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetAnimation, .param = slideAnimationIndex });
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetTexture, .param {.asPtr = slideTexture } });
			break;
		}
		case PState::airborne:
		{
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetAnimation, .param = runAnimationIndex });
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetTexture, .param {.asPtr = runTexture } });
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetGrounded, .param {.asBool = false } });
			break;
		}
	}
	currentState = newState;
}

void StateComponent::update(const FrameContext &ctx)
{
}

void StateComponent::onEvent(int eventId)
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
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::AddImpulse, .param {.asPtr = &jumpImpulse } });
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
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::AddImpulse, .param {.asPtr = &jumpImpulse } });
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
			transitionState(PState::running);
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetGrounded, .param = true });
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
			owner.getCommandDispatch().dispatch(Command{ .id = Commands::AddImpulse, .param {.asPtr = &jumpImpulse } });
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::falling))
		{
			transitionState(PState::airborne);
		}
	}
}
