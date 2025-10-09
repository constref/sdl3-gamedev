#pragma once

#include "component.h"
#include "../timer.h"

struct SDL_Texture;
class SetAnimationMessage;
class DirectionMessage;
class FrameChangeMessage;

class RenderComponent : public Component
{
	Timer flashTimer;
	SDL_Texture *texture;
	bool shouldFlash;
	float width;
	float height;
	int frameNumber;
	float direction;
	float followViewport;

public:
	RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height);
	void update(const FrameContext &ctx) override;
	void onAttached(MessageDispatch &msgDispatch) override;

	void onMessage(const SetAnimationMessage &msg);
	void onMessage(const DirectionMessage &msg);
	void onMessage(const FrameChangeMessage &msg);
	void setTexture(SDL_Texture *texture);
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
};
