#pragma once

#include <vector>
#include  "component.h"
#include "observer.h"

class Animation;

class AnimationComponent : public Component
{
	static const int NO_ANIMATION = -1;

	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;

public:
	AnimationComponent(const std::vector<Animation> &animation);
	void update(GameObject &owner, const FrameContext &ctx) override;
	void setAnimation(int index);

	Subject<int> currentFrameChanged;
};