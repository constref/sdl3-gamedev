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

WeaponSystem::WeaponSystem()
{
	EventQueue::get().dispatcher.registerHandler<ShootBeginEvent>(this);
	EventQueue::get().dispatcher.registerHandler<ShootEndEvent>(this);
	//EventQueue::get().dispatcher.registerHandler<TimerOnTimeout>(this);
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

		World &world = World::get();
		NodeHandle handle = world.createNode();
		Node &bullet = world.getNode(handle);

		auto &res = Resources::get();

		auto &physCmp = bullet.addComponent<PhysicsComponent>();
		const int yVariation = 40;
		const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
		physCmp.setVelocity(glm::vec2(pc->getVelocity().x + 600.0f * pc->getDirection().x, yVelocity));
		physCmp.setMaxSpeed(glm::vec2(1000.0f, 100.0f));
		physCmp.setDynamic(true);
		physCmp.setGravityFactor(0);
		physCmp.setDamping(0);

		auto &animCmp = bullet.addComponent<AnimationComponent>(res.bulletAnims);
		animCmp.setAnimation(res.ANIM_BULLET_MOVING);

		auto &rndCmp = bullet.addComponent<SpriteComponent>(res.texBullet,
			static_cast<float>(res.texBullet->h), static_cast<float>(res.texBullet->h));
		//rndCmp.setDirection(playerDirection);

		auto &collCmp = bullet.addComponent<CollisionComponent>();
		collCmp.setCollider(SDL_FRect{
			.x = 0, .y = 0,
			.w = 4, .h = 4
			});

		bullet.addComponent<ProjectileComponent>();

		// adjust bullet start position
		SDL_FRect collider = collCmp.getCollider();
		const float left = -6;
		const float right = 33;
		const float t = (pc->getDirection().x + 1) / 2.0f; // results in a value of 0..1
		const float xOffset = left + (right - left) * t; // LERP between left and right based on direction

		bullet.setPosition(glm::vec2(
			node.getPosition().x + xOffset,
			node.getPosition().y + 32 / 2
		));

		node.addChild(handle);
	}
}

void WeaponSystem::onEvent(NodeHandle target, const ShootBeginEvent &event)
{
	auto [wc, pc] = getRequiredComponents(World::get().getNode(target));
	if (!wc->isShooting())
	{
		wc->setIsShooting(true);
	}
}

void WeaponSystem::onEvent(NodeHandle target, const ShootEndEvent &event)
{
	auto [wc, pc] = getRequiredComponents(World::get().getNode(target));
	if (wc->isShooting())
	{
		wc->setIsShooting(false);
	}
}
