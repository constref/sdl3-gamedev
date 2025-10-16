#include "basiccameracomponent.h"

#include <gameobject.h>
#include <framecontext.h>
#include <messaging/messages.h>
#include <messaging/messagedispatch.h>

BasicCameraComponent::BasicCameraComponent(GameObject &owner, float viewportWidth, float viewportHeight) : Component(owner)
{
	targetPosition = glm::vec2(0);
	viewportSize = glm::vec2(viewportWidth, viewportHeight);
	velocity = glm::vec2(0);
}

void BasicCameraComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<BasicCameraComponent, VelocityMessage>(this);
}

void BasicCameraComponent::update(const FrameContext &ctx)
{
	targetPosition.x = (owner.getPosition().x + 32 / 2) - viewportSize.x / 2;
	targetPosition.y = 1300;
	//targetPosition.y = res.map->mapHeight * res.map->tileHeight - gs.mapViewport.h;
	owner.sendMessage(ViewportMessage(targetPosition, viewportSize));
}

void BasicCameraComponent::onMessage(const VelocityMessage &msg)
{
	velocity = msg.getVelocity();
}