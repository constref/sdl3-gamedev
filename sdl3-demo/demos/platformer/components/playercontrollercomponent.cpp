#include "playercontrollercomponent.h"

#include <framecontext.h>
#include <messaging/messaging.h>
#include <node.h>
#include <logger.h>

PlayerControllerComponent::PlayerControllerComponent(Node &owner)
	: Component(owner), slideTimer(0.16f)
{
	direction = 0;
	velocity = glm::vec2(0);
	grounded = false;
	shooting = false;
	currentState = PState::idle;
}

void PlayerControllerComponent::onCommand(const UpdateVelocityCommand &msg)
{
	this->velocity = msg.getVelocity();
}

void PlayerControllerComponent::onCommand(const UpdateDirectionCommand &msg)
{
	this->direction = msg.getDirection();
}

