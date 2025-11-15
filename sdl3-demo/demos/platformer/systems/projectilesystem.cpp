#include "projectilesystem.h"
#include <messaging/messaging.h>
#include <resources.h>

ProjectileSystem::ProjectileSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<CollisionEvent>(this);
	services.eventQueue().dispatcher.registerHandler<NodeRemovalEvent>(this);
}

void ProjectileSystem::update(Node &node)
{
}

void ProjectileSystem::onEvent(NodeHandle target, const CollisionEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [rc, pc, cc] = getRequiredComponents(node);

		//TODO: Remove the need to count, group contacts per - frame
		if (!rc->hasHit())
		{
			rc->setHasHit(true);
			Resources &res = Resources::get();
			pc->setVelocity(glm::vec2(0));


			services.eventQueue().enqueue<AnimationPlayEvent>(target, 0,
				res.ANIM_BULLET_HIT, res.texBulletHit, AnimationPlaybackMode::oneShot);
			services.eventQueue().enqueue<RemoveColliderEvent>(target, 0);
			//services.eventQueue().enqueue<DamageEvent>(event.getOther(), 0, owner.getParent(), 15); // damage source is the person firing the gun, not the projectile
			scheduleDestroy(target, res.bulletAnims[res.ANIM_BULLET_HIT].getLength());
		}
	}
}

void ProjectileSystem::onEvent(NodeHandle target, const NodeRemovalEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		Node &parent = services.world().getNode(node.getParent());
		parent.removeChild(target);
		services.world().free(target);
	}
}