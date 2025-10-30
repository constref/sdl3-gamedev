#pragma once

#include <components/rendercomponent.h>

class SpriteRenderComponent : public RenderComponent
{
public:
	SpriteRenderComponent(Node &owner, SDL_Texture *texture, float width, float height)
		: RenderComponent(owner, texture, width, height)
	{
	}
};