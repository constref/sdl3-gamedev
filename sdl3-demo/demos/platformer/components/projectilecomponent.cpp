#include "projectilecomponent.h"

#include <node.h>
#include <world.h>
#include <messaging/events.h>
#include <messaging/eventdispatcher.h>
#include <messaging/commands.h>
#include <messaging/eventqueue.h>
#include <resources.h>
#include <components/collisioncomponent.h>

#include "../events.h"

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, FrameStage::Gameplay)
{
	collisions = 0;

	/*owner.getEventDispatcher().registerHandler<CollisionEvent>(this);
	owner.getEventDispatcher().registerHandler<NodeRemovalEvent>(this);*/
}

void ProjectileComponent::onEvent(const CollisionEvent &event)
{
	// TODO: Remove the need to count, group contacts per-frame
	//collisions++;
	//if (collisions == 1)
	//{
	//	Resources &res = Resources::get();
	//	owner.sendCommand(ScaleVelocityAxisCommand{ Axis::Y, 0 });

	//	services.eventQueue().enqueue<AnimationPlayEvent>(owner.getHandle(), 0,
	//		res.ANIM_BULLET_HIT, res.texBulletHit, AnimationPlaybackMode::oneShot);
	//	services.eventQueue().enqueue<RemoveColliderEvent>(owner.getHandle(), 0);
	//	services.eventQueue().enqueue<DamageEvent>(event.getOther(), 0, owner.getParent(), 15); // damage source is the person firing the gun, not the projectile
	//	owner.scheduleDestroy(res.bulletAnims[res.ANIM_BULLET_HIT].getLength());
	//}
}

void ProjectileComponent::onEvent(const NodeRemovalEvent &event)
{
	//Node &parent = services.world().getNode(owner.getParent());
	//parent.removeChild(owner.getHandle());

	//services.world().free(owner.getHandle());
}
