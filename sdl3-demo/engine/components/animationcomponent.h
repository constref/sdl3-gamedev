#pragma once

#include <vector>
#include <animationplaybackmode.h>
#include "component.h"

class Animation;
class AnimationStopEvent;
class AnimationPlayEvent;

class AnimationComponent : public Component
{
	bool oneShot;
	AnimationPlaybackMode playbackMode;

public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(Node &owner, const std::vector<Animation> &animation);
	void update() override;
	void setAnimation(int index);
	void onEvent(const AnimationStopEvent &event);
	void onEvent(const AnimationPlayEvent &event);
	void setPlaybackMode(AnimationPlaybackMode mode)
	{
		playbackMode = mode;
	}

private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;
};