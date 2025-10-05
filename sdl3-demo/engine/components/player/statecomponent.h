#pragma once

#include <glm/glm.hpp>
#include "../component.h"

struct SDL_Texture;

class StateComponent : public Component
{

public:
	StateComponent(GameObject &owner);
	~StateComponent() override {}

	void transitionState(PState newState);
	void update(const FrameContext &ctx) override;
	void registerObservers(SubjectRegistry &registry) override;
	void onEvent(int eventId) override;

	void setIdleAnimation(int index) { idleAnimationIndex = index; }
	void setIdleTexture(SDL_Texture *tex) { idleTexture = tex; }
	void setRunAnimation(int index) { runAnimationIndex = index; }
	void setRunTexture(SDL_Texture *tex) { runTexture = tex; }
	void setSlideAnimation(int index) { slideAnimationIndex = index; }
	void setSlideTexture(SDL_Texture *tex) { slideTexture = tex; }
};