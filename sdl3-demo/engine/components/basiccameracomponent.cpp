#include "basiccameracomponent.h"

#include <node.h>
#include <framecontext.h>
#include <resources.h>
#include <messaging/commands.h>
#include <messaging/commanddispatcher.h>
#include <world.h>

BasicCameraComponent::BasicCameraComponent(Node &owner, NodeHandle target, float viewportWidth, float viewportHeight) : Component(owner, ComponentStage::Gameplay)
{
	this->target = target;
	camPosition = glm::vec2(0);
	viewportSize = glm::vec2(viewportWidth, viewportHeight);
	velocity = glm::vec2(0);
}

void BasicCameraComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	dataDispatcher.registerHandler<BasicCameraComponent, UpdateVelocityCommand>(this);
}

void BasicCameraComponent::update(const FrameContext &ctx)
{
	Node &obj = World::get().getNode(target);
	const Resources &res = Resources::get();
	camPosition.x = (obj.getPosition().x + res.map->tileWidth / 2) - viewportSize.x / 2;
	camPosition.y = res.map->mapHeight * res.map->tileHeight - viewportSize.y;

	owner.broadcastMessage(UpdateViewportCommand(camPosition, viewportSize));
}

void BasicCameraComponent::onCommand(const UpdateVelocityCommand &msg)
{
	velocity = msg.getVelocity();
}