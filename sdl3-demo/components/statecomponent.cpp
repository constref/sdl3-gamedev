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
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetAnimation, .param = res.ANIM_PLAYER_IDLE });
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetTexture, .param {.asPtr = res.texIdle } });
			break;
		}
		case PState::running:
		{
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetAnimation, .param = res.ANIM_PLAYER_RUN });
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetTexture, .param {.asPtr = res.texRun } });
			break;
		}
		case PState::airborne:
		{
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetAnimation, .param = res.ANIM_PLAYER_RUN });
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetTexture, .param { .asPtr = res.texRun } });
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetGrounded, .param { .asBool = false } });
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
			owner.getCommandDispatch().submit(Command{ .id = Commands::AddImpulse, .param { .asPtr = &jumpImpulse } });
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
			owner.getCommandDispatch().submit(Command{ .id = Commands::AddImpulse, .param { .asPtr = &jumpImpulse } });
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::falling))
		{
			transitionState(PState::airborne);
		}
		else if (eventId == static_cast<int>(Events::slide))
		{
		}
	}
	else if (currentState == PState::airborne)
	{
		if (eventId == static_cast<int>(Events::landed))
		{
			transitionState(PState::idle);
			owner.getCommandDispatch().submit(Command{ .id = Commands::SetGrounded, .param = true });
		}
	}
}
