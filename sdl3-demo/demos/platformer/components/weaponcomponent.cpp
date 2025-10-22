#include "weaponcomponent.h"

#include <iostream>
#include <framecontext.h>
#include <sdlstate.h>
#include <resources.h>
#include <messaging/messagedispatch.h>
#include <messaging/messages.h>
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

void WeaponComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<WeaponComponent, VelocityMessage>(this);
	msgDispatch.registerHandler<WeaponComponent, DirectionMessage>(this);
	msgDispatch.registerHandler<WeaponComponent, ShootStartMessage>(this);
	msgDispatch.registerHandler<WeaponComponent, ShootEndMessage>(this);
}

void WeaponComponent::onMessage(const VelocityMessage &msg)
{
	playerVelocity = msg.getVelocity();
}

void WeaponComponent::onMessage(const DirectionMessage &msg)
{
	if (msg.getDirection())
	{
		playerDirection = msg.getDirection();
	}
}

void WeaponComponent::onMessage(const ShootStartMessage &msg)
{
	if (!shooting)
	{
		shooting = true;
	}
}

void WeaponComponent::onMessage(const ShootEndMessage &msg)
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

