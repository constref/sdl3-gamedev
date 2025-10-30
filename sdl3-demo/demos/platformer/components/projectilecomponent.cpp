#include "projectilecomponent.h"

#include <node.h>
#include <world.h>
#include <messaging/events.h>
#include <messaging/eventdispatcher.h>
#include <messaging/commands.h>
#include <messaging/eventqueue.h>
#include <resources.h>
#include <components/collisioncomponent.h>

#include "events.h"

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, ComponentStage::Gameplay)
{
	collisions = 0;

	owner.getEventDispatcher().registerHandler<CollisionEvent>(this);
	owner.getEventDispatcher().registerHandler<NodeRemovalEvent>(this);
	owner.getEventDispatcher().registerHandler<AnimationEndEvent>(this);
}

void ProjectileComponent::onEvent(const CollisionEvent &event)
{
	// TODO: Remove the need to count, group contacts per-frame
	collisions++;
	if (collisions == 1)
	{
		Resources &res = Resources::get();
		owner.sendCommand(SetAnimationCommand{ res.ANIM_BULLET_HIT, res.texBulletHit, true });
		owner.sendCommand(ScaleVelocityAxisCommand{ Axis::Y, 0 });

		EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), ComponentStage::PostRender);
		EventQueue::get().enqueue<DamageEvent>(event.getOther(), ComponentStage::Gameplay, 15);
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
