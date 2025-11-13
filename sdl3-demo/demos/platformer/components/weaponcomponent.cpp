#include "weaponcomponent.h"

#include <iostream>
#include <framecontext.h>
#include <sdlstate.h>
#include <resources.h>
#include <messaging/commanddispatcher.h>
#include <messaging/commands.h>
#include <messaging/events.h>
#include <world.h>

#include <components/spritecomponent.h>
#include <components/animationcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>

#include "projectilecomponent.h"

WeaponComponent::WeaponComponent(Node &owner) : Component(owner, FrameStage::Gameplay), cooldownTimer(0.1f)
{
	shooting = false;
	canFire = true;
	playerDirection = 1;
	playerVelocity = glm::vec2(0);
}

