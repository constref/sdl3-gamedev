#include "basiccameracomponent.h"

#include <gameobject.h>
#include <framecontext.h>
#include <resources.h>
#include <messaging/messages.h>
#include <messaging/messagedispatch.h>

BasicCameraComponent::BasicCameraComponent(GameObject &owner, std::shared_ptr<GameObject> target, float viewportWidth, float viewportHeight) : Component(owner)
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
	const Resources &res = Resources::getInstance();
	camPosition.x = (target->getPosition().x + 32 / 2) - viewportSize.x / 2;
	camPosition.y = res.map->mapHeight * res.map->tileHeight - viewportSize.y;
	owner.sendMessage(ViewportMessage(camPosition, viewportSize));
}

void BasicCameraComponent::onMessage(const VelocityMessage &msg)
{
	velocity = msg.getVelocity();
}