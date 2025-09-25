#pragma once

#include "../component.h"
#include "../timer.h"

struct SDL_Texture;
class AnimationComponent;
class InputComponent;

class RenderComponent : public Component
{
	Timer flashTimer;
	SDL_Texture *texture;
	bool shouldFlash;
	float width;
	float height;
	int frameNumber;
	float direction;

public:
	RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height,
		AnimationComponent *animComponent = nullptr, InputComponent *inputComponent = nullptr);
	void update(const FrameContext &ctx) override;
	void onAttached() override;
	void onCommand(const Command &command) override;

	void setTexture(SDL_Texture *texture);
};
