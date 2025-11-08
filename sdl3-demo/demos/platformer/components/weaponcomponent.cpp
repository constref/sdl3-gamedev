#include "weaponcomponent.h"

#include <iostream>
#include <framecontext.h>
#include <sdlstate.h>
#include <resources.h>
#include <messaging/commanddispatcher.h>
#include <messaging/commands.h>
#include <messaging/events.h>
#include <world.h>

#include <components/rendercomponent.h>
#include <components/animationcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

#include "projectilecomponent.h"

WeaponComponent::WeaponComponent(Node &owner) : Component(owner, FrameStage::Gameplay), timer(0.1f)
{
	shooting = false;
	playerDirection = 1;
	playerVelocity = glm::vec2(0);

	owner.getCommandDispatcher().registerHandler<UpdateVelocityCommand>(this);
	owner.getCommandDispatcher().registerHandler<UpdateDirectionCommand>(this);
	owner.getEventDispatcher().registerHandler<ShootBeginEvent>(this);
	owner.getEventDispatcher().registerHandler<ShootEndEvent>(this);
}

void WeaponComponent::onCommand(const UpdateVelocityCommand &dp)
{
	playerVelocity = dp.getVelocity();
}

void WeaponComponent::onCommand(const UpdateDirectionCommand &dp)
{
	if (dp.getDirection())
	{
		playerDirection = dp.getDirection();
	}
}

void WeaponComponent::onEvent(const ShootBeginEvent &event)
{
	if (!shooting)
	{
		shooting = true;
	}
}

void WeaponComponent::onEvent(const ShootEndEvent &event)
{
	if (shooting)
	{
		shooting = false;
	}
}

void WeaponComponent::update()
{
	timer.step(FrameContext::global().deltaTime);

	if (shooting)
	{
		if (timer.getTimeouts())
		{
			timer.reset();
			World &world = World::get();
			NodeHandle handle = world.createNode();
			Node &bullet = world.getNode(handle);

			auto &res = Resources::get();

			auto &physCmp = bullet.addComponent<PhysicsComponent>();
			const int yVariation = 40;
			const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
			physCmp.setVelocity(glm::vec2(playerVelocity.x + 600.0f * playerDirection, yVelocity));
			physCmp.setMaxSpeed(glm::vec2(1000.0f, 100.0f));
			physCmp.setDynamic(true);
			physCmp.setGravityFactor(0);
			physCmp.setDamping(0);

			auto &animCmp = bullet.addComponent<AnimationComponent>(res.bulletAnims);
			animCmp.setAnimation(res.ANIM_BULLET_MOVING);

			auto &rndCmp = bullet.addComponent<RenderComponent>(res.texBullet,
				static_cast<float>(res.texBullet->h), static_cast<float>(res.texBullet->h));
			rndCmp.setDirection(playerDirection);

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
			const float t = (playerDirection + 1) / 2.0f; // results in a value of 0..1
			const float xOffset = left + (right - left) * t; // LERP between left and right based on direction

			bullet.setPosition(glm::vec2(
				owner.getPosition().x + xOffset,
				owner.getPosition().y + 32 / 2
			));

			owner.addChild(handle);
		}
	}
}

