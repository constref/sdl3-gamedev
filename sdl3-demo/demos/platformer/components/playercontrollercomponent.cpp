#include "playercontrollercomponent.h"

#include <framecontext.h>
#include <messaging/messaging.h>
#include <node.h>
#include <logger.h>

PlayerControllerComponent::PlayerControllerComponent(Node &owner)
	: Component(owner, FrameStage::Gameplay), slideTimer(0.16f)
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
	jumpAnimationIndex = 0;
	jumpTexture = nullptr;
	slideAnimationIndex = 0;
	slideTexture = nullptr;
	shootAnimationIndex = 0;
	shootTexture = nullptr;

	owner.getCommandDispatcher().registerHandler<UpdateVelocityCommand>(this);
	owner.getCommandDispatcher().registerHandler<UpdateDirectionCommand>(this);
}

void PlayerControllerComponent::onCommand(const UpdateVelocityCommand &msg)
{
	this->velocity = msg.getVelocity();
}

void PlayerControllerComponent::onCommand(const UpdateDirectionCommand &msg)
{
	this->direction = msg.getDirection();
}

