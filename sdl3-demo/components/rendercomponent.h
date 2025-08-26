#pragma once

#include "component.h"
#include "../timer.h"

struct SDL_Texture;
class AnimationComponent;

class RenderComponent : public Component
{
	Timer flashTimer;
	SDL_Texture *texture;
	bool shouldFlash;
	float width;
	float height;
	int frameNumber;

public:
	RenderComponent(SDL_Texture *texture, float width, float height, AnimationComponent *animComponent, GameObject &owner);
	void update(SDLState &state, GameState &gs, Resources &res, float deltaTime) override;

	void setTexture(SDL_Texture *texture);
};
