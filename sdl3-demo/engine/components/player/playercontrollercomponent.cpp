#include "playercontrollercomponent.h"

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


void PlayerControllerComponent::transitionState(PState newState)
{
	// on enter actions
	switch (newState)
	{
		case PState::idle:
		{
			owner.sendMessage(SetAnimationMessage { idleAnimationIndex, idleTexture });
			break;
		}
		case PState::running:
		{
			owner.sendMessage(SetAnimationMessage { runAnimationIndex, runTexture });
			break;
		}
		case PState::sliding:
		{
			owner.sendMessage(SetAnimationMessage { slideAnimationIndex, slideTexture });
			break;
		}
		case PState::airborne:
		{
			owner.sendMessage(SetAnimationMessage { runAnimationIndex, runTexture });
			//owner.getCommandDispatch().dispatch(Command{ .id = Commands::SetGrounded, .param {.asBool = false } });
			break;
		}
	}
	currentState = newState;
}
void PlayerControllerComponent::update(const FrameContext &ctx)
{
	if (!direction)
	{
		// decelerate
		const float factor = velocity.x > 0 ? -1.5f : 1.5f;
		//float amount = factor * acceleration.x * ctx.deltaTime;
		float amount = 0;
		if (std::abs(velocity.x) < std::abs(amount) && velocity.x != 0)
		{
			//owner.getCommandDispatch().dispatch(Command{ .id = Commands::ZeroVelocityX });
			emit(ctx, static_cast<int>(Events::idle));
		}
		else
		{
			//vel.x += amount;
		}
		//	emit(ctx, static_cast<int>(Events::idle));
	}
	// check if sliding (direction and velocity signs are different)
	//if (direction)
	//{
	//	if (!isRunning)
	//	{
	//		isRunning = true;
	//		emit(ctx, static_cast<int>(Events::run));
	//	}

	//	bool wasSliding = isSliding;
	//	isSliding = direction * velocity.x < 0;
	//	if (isSliding)
	//	{
	//		if (isSliding != wasSliding)
	//		{
	//			emit(ctx, static_cast<int>(Events::slide));
	//		}
	//	}
	//}
	//else
	//{
	//}
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
			//owner.getCommandDispatch().dispatch(Command{ .id = Commands::AddImpulse, .param {.asPtr = &jumpImpulse } });
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
			//owner.getCommandDispatch().dispatch(Command{ .id = Commands::AddImpulse, .param {.asPtr = &jumpImpulse } });
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

void PlayerControllerComponent::onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch)
{
	//owner.getCommandDispatch().registerCommand(Commands::Jump, this);
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
