#include "projectilecomponent.h"

#include <node.h>
#include <messaging/events.h>
#include <messaging/eventdispatcher.h>
#include <resources.h>

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, ComponentStage::Gameplay)
{
}

void ProjectileComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<ProjectileComponent, CollisionEvent>(this);
}

void ProjectileComponent::onEvent(const CollisionEvent &event)
{
	Resources &res = Resources::get();
}

void ProjectileComponent::update(const FrameContext &ctx)
{
}
