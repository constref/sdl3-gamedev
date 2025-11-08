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
}

void ProjectileComponent::onEvent(const CollisionEvent &event)
{
	// TODO: Remove the need to count, group contacts per-frame
	collisions++;
	if (collisions == 1)
	{
		Resources &res = Resources::get();
		owner.sendCommand(ScaleVelocityAxisCommand{ Axis::Y, 0 });

		EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, res.ANIM_BULLET_HIT, res.texBulletHit, AnimationPlaybackMode::oneShot);
		EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), ComponentStage::PostRender, 0);
		EventQueue::get().enqueue<DamageEvent>(event.getOther(), ComponentStage::Gameplay, 0, 15);
		owner.scheduleDestroy(res.bulletAnims[res.ANIM_BULLET_HIT].getLength());

		// apply push force from projectile
		Node &other = World::get().getNode(event.getOther());
		other.sendCommand<AddImpulseCommand>(AddImpulseCommand{ glm::vec2(200, 0) });
	}
}

void ProjectileComponent::onEvent(const NodeRemovalEvent &event)
{
	Node &parent = World::get().getNode(owner.getParent());
	parent.removeChild(owner.getHandle());

	World::get().free(owner.getHandle());
}

void ProjectileComponent::onCommand(const UpdateViewportCommand &dp)
{
	//mapViewportPos.x = dp.getPosition().x;
	//mapViewportPos.y = dp.getPosition().y;
	//mapViewportSize.x = dp.getSize().x;
	//mapViewportSize.y = dp.getSize().y;
}
