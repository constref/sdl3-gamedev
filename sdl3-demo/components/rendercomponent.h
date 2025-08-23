#pragma once

#include "component.h"
#include "../timer.h"

struct SDL_Texture;

class RenderComponent : public Component
{
	Timer flashTimer;
	SDL_Texture *texture;
	bool shouldFlash;
	float width;
	float height;

public:
	RenderComponent(SDL_Texture *texture, float width, float height, GameObject &owner) : Component(owner), flashTimer(0.05f)
	{
		this->texture = texture;
		shouldFlash = false;
		this->width = width;
		this->height = height;
	}
	void update(SDLState &state, GameState &gs, Resources &res, float deltaTime) override;

	void setTexture(SDL_Texture *texture);
};
