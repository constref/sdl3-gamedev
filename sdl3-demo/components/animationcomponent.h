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
	AnimationComponent(std::shared_ptr<GameObject> owner, const std::vector<Animation> &animation);
	void update(const FrameContext &ctx) override;
	void setAnimation(int index);

	Subject<int> currentFrameChanged;
};