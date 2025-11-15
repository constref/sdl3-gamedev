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
	//services.eventQueue().dispatcher.registerHandler<TimerOnTimeout>(this);
}

void WeaponSystem::update(Node &node)
{
	auto [wc, pc] = getRequiredComponents(node);

	bool canFire = wc->getCooldownTimer().step(FrameContext::dt());
	if (wc->isShooting() && canFire)
	{
		if (pc->getDirection().x != 0)
		{
			fireDirection = pc->getDirection();
		}

		// restart cooldown timer
		wc->getCooldownTimer().reset();
		canFire = false;

		World &world = services.world();
		NodeHandle handle = world.createNode();
		Node &bullet = world.getNode(handle);
		bullet.setTag(4);

		auto &res = Resources::get();

		auto &physCmp = services.compSys().addComponent<PhysicsComponent>(bullet);
		const int yVariation = 40;
		const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
		physCmp.setVelocity(glm::vec2(pc->getVelocity().x + 600.0f * fireDirection.x, yVelocity));
		physCmp.setMaxSpeed(glm::vec2(1000.0f, 100.0f));
		physCmp.setDynamic(true);
		physCmp.setGravityFactor(0);
		physCmp.setDamping(0);

		auto &animCmp = services.compSys().addComponent<AnimationComponent>(bullet, res.bulletAnims);
		animCmp.setAnimation(res.ANIM_BULLET_MOVING);
		auto &rndCmp = services.compSys().addComponent<SpriteComponent>(
			bullet, res.texBullet, static_cast<float>(res.texBullet->h),
			static_cast<float>(res.texBullet->h));

		auto &collCmp = services.compSys().addComponent<CollisionComponent>(bullet);
		collCmp.setCollider(SDL_FRect{
			.x = 0, .y = 0,
			.w = 4, .h = 4
		});
		services.compSys().addComponent<ProjectileComponent>(bullet);

		// adjust bullet start position
		SDL_FRect collider = collCmp.getCollider();
		const float left = -6;
		const float right = 33;
		const float t = (fireDirection.x + 1) / 2.0f; // results in a value of 0..1
		const float xOffset = left + (right - left) * t; // LERP between left and right based on direction

		bullet.setPosition(glm::vec2(
			node.getPosition().x + xOffset,
			node.getPosition().y + 32 / 2
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
