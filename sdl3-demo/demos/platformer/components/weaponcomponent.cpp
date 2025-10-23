#include "weaponcomponent.h"

#include <iostream>
#include <framecontext.h>
#include <sdlstate.h>
#include <resources.h>
#include <messaging/datadispatcher.h>
#include <messaging/datapumps.h>
#include <world.h>

#include <components/rendercomponent.h>
#include <components/animationcomponent.h>
#include <components/physicscomponent.h>

WeaponComponent::WeaponComponent(GameObject &owner) : Component(owner, ComponentStage::Gameplay), timer(0.1f)
{
	shooting = false;
	playerDirection = 1;
	playerVelocity = glm::vec2(0);
}

void WeaponComponent::onAttached(DataDispatcher &dataDispatcher)
{
	dataDispatcher.registerHandler<WeaponComponent, VelocityDPump>(this);
	dataDispatcher.registerHandler<WeaponComponent, DirectionDPump>(this);
	dataDispatcher.registerHandler<WeaponComponent, ShootStartDPump>(this);
	dataDispatcher.registerHandler<WeaponComponent, ShootEndDPump>(this);
}

void WeaponComponent::onData(const VelocityDPump &dp)
{
	playerVelocity = dp.getVelocity();
}

void WeaponComponent::onData(const DirectionDPump &dp)
{
	if (dp.getDirection())
	{
		playerDirection = dp.getDirection();
	}
}

void WeaponComponent::onData(const ShootStartDPump &dp)
{
	if (!shooting)
	{
		shooting = true;
	}
}

void WeaponComponent::onData(const ShootEndDPump &dp)
{
	if (shooting)
	{
		shooting = false;
	}
}

void WeaponComponent::update(const FrameContext &ctx)
{
	timer.step(ctx.deltaTime);

	if (shooting)
	{
		if (timer.isTimeout())
		{
			timer.reset();
			World &world = World::getInstance();
			GHandle handle = world.createObject();
			GameObject &bullet = world.getObject(handle);

			auto &res = Resources::getInstance();

			auto &physCmp = bullet.addComponent<PhysicsComponent>();
			const int yVariation = 40;
			const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
			physCmp.setVelocity(glm::vec2(playerVelocity.x + 600.0f * playerDirection, yVelocity));
			physCmp.setMaxSpeed(glm::vec2(1000.0f, 100.0f));

			auto &animCmp = bullet.addComponent<AnimationComponent>(res.bulletAnims);
			animCmp.setAnimation(AnimationComponent::NO_ANIMATION);

			bullet.addComponent<RenderComponent>(res.texBullet,
				static_cast<float>(res.texBullet->w), static_cast<float>(res.texBullet->h));

			// adjust bullet start position
			const float left = 4;
			const float right = 24;
			const float t = (playerDirection + 1) / 2.0f; // results in a value of 0..1
			const float xOffset = left + right * t; // LERP between left and right based on direction
			bullet.setPosition(glm::vec2(
				owner.getPosition().x + xOffset,
				owner.getPosition().y + 32 / 2 + 1
			));

			owner.addChild(handle);
		}
	}
}

