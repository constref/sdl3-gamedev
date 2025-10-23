#include "basiccameracomponent.h"

#include <gameobject.h>
#include <framecontext.h>
#include <resources.h>
#include <messaging/datapumps.h>
#include <messaging/datadispatcher.h>
#include <world.h>

BasicCameraComponent::BasicCameraComponent(GameObject &owner, GHandle target, float viewportWidth, float viewportHeight) : Component(owner, ComponentStage::Gameplay)
{
	this->target = target;
	camPosition = glm::vec2(0);
	viewportSize = glm::vec2(viewportWidth, viewportHeight);
	velocity = glm::vec2(0);
}

void BasicCameraComponent::onAttached(DataDispatcher &dataDispatcher)
{
	dataDispatcher.registerHandler<BasicCameraComponent, VelocityDPump>(this);
}

void BasicCameraComponent::update(const FrameContext &ctx)
{
	GameObject &obj = World::getInstance().getObject(target);
	const Resources &res = Resources::getInstance();
	camPosition.x = (obj.getPosition().x + res.map->tileWidth / 2) - viewportSize.x / 2;
	camPosition.y = res.map->mapHeight * res.map->tileHeight - viewportSize.y;

	owner.broadcastMessage(ViewportDPump(camPosition, viewportSize));
}

void BasicCameraComponent::onData(const VelocityDPump &msg)
{
	velocity = msg.getVelocity();
}