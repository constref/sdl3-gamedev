#pragma once

#include "../component.h"
#include "../timer.h"

struct SDL_Texture;
class SetAnimationMessage;

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
	RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height);
	void update(const FrameContext &ctx) override;
	void onAttached(SubjectRegistry &registry) override;
	void registerObservers(SubjectRegistry &registry) override;

	void onMessage(const SetAnimationMessage &msg);
	void setTexture(SDL_Texture *texture);
};
