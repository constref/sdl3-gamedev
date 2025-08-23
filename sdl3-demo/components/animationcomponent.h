#pragma once

#include <vector>
#include  "component.h"

class Animation;

class AnimationComponent : public Component
{
	static const int NO_ANIMATION = -1;

	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;

public:
	AnimationComponent(const std::vector<Animation> &animation, GameObject &owner);
	void update(SDLState &state, GameState &gs, Resources &res, float deltaTime) override;
	void setAnimation(int index);
};