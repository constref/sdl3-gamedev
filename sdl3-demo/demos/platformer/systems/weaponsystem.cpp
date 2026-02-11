#include "weaponsystem.h"

#include "../components/weaponcomponent.h"
#include "../components/projectilecomponent.h"

#include <resources.h>
#include <framecontext.h>
#include <components/animationcomponent.h>
#include <components/collisioncomponent.h>
#include <components/spritecomponent.h>
#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <componentsystems.h>
#include <logger.h>

WeaponSystem::WeaponSystem(Services &services) : System(services)
{
	fireDirection = { 1, 0 };
	services.eventQueue().dispatcher.registerHandler<ShootBeginEvent>(this);
	services.eventQueue().dispatcher.registerHandler<ShootEndEvent>(this);
	services.eventQueue().dispatcher.registerHandler<DirectionChangedEvent>(this);
	//services.eventQueue().dispatcher.registerHandler<TimerTimeoutEvent>(this);
}

void WeaponSystem::update(Node &node)
{
	auto [wc, pc] = getRequiredComponents(node);

	bool canFire = wc->getCooldownTimer().step(FrameContext::dt());
	if (wc->isShooting() && canFire)
	{
		// restart cooldown timer
		wc->getCooldownTimer().reset();
		canFire = false;

		World &world = services.world();
		NodeHandle handle = world.createNode();
		Node &bullet = world.getNode(handle);
		bullet.setTag(4);

		auto &physCmp = services.compSys().addComponent<PhysicsComponent>(bullet);
		const int yVariation = 40;
		const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
		physCmp.setVelocity(glm::vec3(pc->getVelocity().x + 600.0f * fireDirection.x, yVelocity, 0));
		physCmp.setMaxSpeed(glm::vec3(1000.0f, 100.0f, 0));
		//physCmp.setDynamic(true);
		physCmp.setGravityFactor(0);
		physCmp.setDamping(0);

		services.compSys().addComponent<AnimationComponent>(bullet);
		services.eventQueue().enqueue<AnimationPlayEvent>(handle, 0, wc->animProjectile, wc->texProjectile, AnimationPlaybackMode::continuous);
		auto &rndCmp = services.compSys().addComponent<SpriteComponent>(
			bullet, wc->texProjectile, 4.0f, 4.0f);
		rndCmp.setFlipH(fireDirection.x < 0);
		rndCmp.setRotation(static_cast<float>(SDL_rand(360)));

		auto &collCmp = services.compSys().addComponent<CollisionComponent>(bullet);
		collCmp.setCollider(Collider {
			.x = 0, .y = 0,
			.w = 4, .h = 4
			});
		auto &projComp = services.compSys().addComponent<ProjectileComponent>(bullet);
		projComp.animProjectileHitId = wc->animProjectileHit;
		projComp.texProjectileHitId = wc->texProjectileHit;

		// adjust bullet start position
		Collider collider = collCmp.getCollider();
		const float left = -6;
		const float right = 33;
		const float t = (fireDirection.x + 1) / 2.0f; // results in a value of 0..1
		const float xOffset = left + (right - left) * t; // LERP between left and right based on direction

		bullet.setPosition(glm::vec3(
			node.getPosition().x + xOffset,
			node.getPosition().y + 32 / 2,
			0
		));

		node.addChild(bullet);
	}
}

void WeaponSystem::onEvent(NodeHandle target, const ShootBeginEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [wc, pc] = getRequiredComponents(node);
		if (!wc->isShooting())
		{
			wc->setIsShooting(true);
		}
	}
}

void WeaponSystem::onEvent(NodeHandle target, const ShootEndEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [wc, pc] = getRequiredComponents(node);
		if (wc->isShooting())
		{
			wc->setIsShooting(false);
		}
	}
}

void WeaponSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event)
{
	// we only care if we're turning (-1, 1)
	if (event.getDirection().x != 0)
	{
		fireDirection = event.getDirection();
	}
}
