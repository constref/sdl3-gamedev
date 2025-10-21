#include "weaponcomponent.h"

#include <framecontext.h>
#include <sdlstate.h>
#include <messaging/messagedispatch.h>
#include <messaging/messages.h>
#include <SDL3/SDL.h>

WeaponComponent::WeaponComponent(GameObject &owner) : Component(owner)
{
	shooting = false;
}

void WeaponComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<WeaponComponent, ShootStartMessage>(this);
	msgDispatch.registerHandler<WeaponComponent, ShootEndMessage>(this);
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
	if (shooting)
	{
		SDL_SetWindowTitle(ctx.state.window, "SHOOTING");
	}
}

