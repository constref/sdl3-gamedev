#include "projectilecomponent.h"

#include <node.h>
#include <world.h>
#include <messaging/events.h>
#include <messaging/eventdispatcher.h>
#include <resources.h>

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, ComponentStage::Gameplay)
{
	collisions = 0;
}

void ProjectileComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<ProjectileComponent, CollisionEvent>(this);
	eventDispatcher.registerHandler<ProjectileComponent, NodeRemovalEvent>(this);
}

void ProjectileComponent::onEvent(const CollisionEvent &event)
{
	collisions++;
	if (collisions == 1)
	{
		owner.destroySelf();
	}
}

void ProjectileComponent::update(const FrameContext &ctx)
{
}

void ProjectileComponent::onEvent(const NodeRemovalEvent &event)
{
	Node &parent = World::get().getNode(owner.getParent());
	parent.removeChild(owner.getHandle());

	World::get().free(owner.getHandle());
}
