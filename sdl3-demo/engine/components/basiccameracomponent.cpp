#include "basiccameracomponent.h"

#include <gameobject.h>
#include <framecontext.h>
#include <resources.h>
#include <messaging/messages.h>
#include <messaging/messagedispatch.h>
#include <world.h>

BasicCameraComponent::BasicCameraComponent(GameObject &owner, GHandle target, float viewportWidth, float viewportHeight) : Component(owner)
{
	this->target = target;
	camPosition = glm::vec2(0);
	viewportSize = glm::vec2(viewportWidth, viewportHeight);
	velocity = glm::vec2(0);
}

void BasicCameraComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<BasicCameraComponent, VelocityMessage>(this);
}

void BasicCameraComponent::update(const FrameContext &ctx)
{
	GameObject &obj = World::getInstance().getObject(target);
	const Resources &res = Resources::getInstance();
	camPosition.x = (obj.getPosition().x + res.map->tileWidth / 2) - viewportSize.x / 2;
	camPosition.y = res.map->mapHeight * res.map->tileHeight - viewportSize.y;

	owner.broadcastMessage(ViewportMessage(camPosition, viewportSize));
}

void BasicCameraComponent::onMessage(const VelocityMessage &msg)
{
	velocity = msg.getVelocity();
}