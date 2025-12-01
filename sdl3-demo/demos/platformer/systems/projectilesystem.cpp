#include "projectilesystem.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <componentsystems.h>
#include "../events.h"

ProjectileSystem::ProjectileSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<CollisionEvent>(this);
	services.eventQueue().dispatcher.registerHandler<NodeRemovalEvent>(this);
}

void ProjectileSystem::update(Node &node)
{
	// TODO: Replace this with feature in anim system
	auto *projComp = node.getComponent<ProjectileComponent>();
	if (!projComp->hasHit())
	{
		const float maxLife = 0.5f;
		float life = std::clamp(projComp->getLifeDuration() + FrameContext::dt(), 0.0f, maxLife);
		projComp->setLifeDuration(life);

		auto *spriteComp = node.getComponent<SpriteComponent>();
		float norm = life / maxLife;

		float s = 0.8f + (1.2f - 0.2f) * norm;
		spriteComp->setScale(glm::vec2(s));
	}
}

void ProjectileSystem::onEvent(NodeHandle target, const CollisionEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [rc, pc, cc, sc] = getRequiredComponents(node);
		if (!rc->hasHit())
		{
			rc->setHasHit(true);
			Resources &res = Resources::get();
			pc->setVelocity(glm::vec2(0));

			services.eventQueue().enqueue<AnimationPlayEvent>(target, 0,
				res.ANIM_BULLET_HIT, res.texBulletHit, AnimationPlaybackMode::oneShot);
			services.eventQueue().enqueue<DamageEvent>(event.getOther(), 0, node.getParent(), 15); // damage source is the person firing the gun, not the projectile
			sc->setRotation(0);

			scheduleDestroy(target, res.bulletAnims[res.ANIM_BULLET_HIT].getLength());
			services.compSys().removeComponent(node, *cc);
		}
	}
}

void ProjectileSystem::onEvent(NodeHandle target, const NodeRemovalEvent &event)
{
	Node &node = services.world().getNode(target);
	Node &parent = services.world().getNode(node.getParent());
	parent.removeChild(target);
	services.world().free(target);
}