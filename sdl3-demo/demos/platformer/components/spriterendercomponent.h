#pragma once

#include <components/rendercomponent.h>
#include <messaging/eventdispatcher.h>
#include "events.h"

class DamageEvent;

class SpriteRenderComponent : public RenderComponent
{
public:
	SpriteRenderComponent(Node &owner, SDL_Texture *texture, float width, float height)
		: RenderComponent(owner, texture, width, height)
	{
		owner.getEventDispatcher().registerHandler<DamageEvent>(this);
	}

	void onEvent(const DamageEvent &event)
	{
		shouldFlash = true;
	}
};