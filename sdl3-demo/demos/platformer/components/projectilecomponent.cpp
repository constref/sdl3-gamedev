#include "projectilecomponent.h"

#include <node.h>
#include <world.h>
#include <messaging/events.h>
#include <messaging/eventdispatcher.h>
#include <messaging/commands.h>
#include <messaging/eventqueue.h>
#include <resources.h>
#include <components/collisioncomponent.h>

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, ComponentStage::Gameplay)
{
	collisions = 0;
}

void ProjectileComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<ProjectileComponent, CollisionEvent>(this);
	eventDispatcher.registerHandler<ProjectileComponent, NodeRemovalEvent>(this);
	eventDispatcher.registerHandler<ProjectileComponent, AnimationEndEvent>(this);
}

void ProjectileComponent::onEvent(const CollisionEvent &event)
{
	collisions++;
	if (collisions == 1)
	{
		Resources &res = Resources::get();
		owner.pushData(SetAnimationCommand{ res.ANIM_BULLET_HIT, res.texBulletHit, true });
		owner.pushData(ScaleVelocityAxisCommand{ Axis::Y, 0 });

		EventQueue::get().enqueue<RemoveCollisionEvent>(owner.getHandle(), ComponentStage::Physics);
	}
}

void ProjectileComponent::onEvent(const NodeRemovalEvent &event)
{
	Node &parent = World::get().getNode(owner.getParent());
	parent.removeChild(owner.getHandle());

	World::get().free(owner.getHandle());
}

void ProjectileComponent::onEvent(const AnimationEndEvent &event)
{
	Resources &res = Resources::get();
	if (event.getIndex() == res.ANIM_BULLET_HIT)
	{
		owner.scheduleDestroy();
	}
}
