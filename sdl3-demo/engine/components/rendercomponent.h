#pragma once

#include "component.h"
#include <SDL3/SDL.h>
#include "../timer.h"

struct SDL_Texture;
class SetAnimationDPump;
class DirectionDPump;
class FrameChangeDPump;
class ViewportDPump;

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
	static SDL_FRect mapViewport;

public:
	RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height);
	void update(const FrameContext &ctx) override;
	void onAttached(DataDispatcher &dataDispatcher) override;

	void onData(const SetAnimationDPump &dp);
	void onData(const DirectionDPump &dp);
	void onData(const FrameChangeDPump &dp);
	void onData(const ViewportDPump &dp);

	void setTexture(SDL_Texture *texture);
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
};
