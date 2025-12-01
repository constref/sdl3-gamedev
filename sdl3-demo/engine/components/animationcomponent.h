#pragma once

#include <vector>
#include <animationplaybackmode.h>
#include "component.h"

class Animation;

class AnimationComponent : public Component
{
	bool oneShot;
	AnimationPlaybackMode playbackMode;

public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(Node &owner, const std::vector<Animation> &animation);
	void update() override;
	int getAnimation() const;
	void setAnimation(int index);
	AnimationPlaybackMode getPlaybackMode() const { return playbackMode; }
	void setPlaybackMode(AnimationPlaybackMode mode)
	{
		playbackMode = mode;
	}

	int getFrameNumber() const { return frameNumber; }
	auto &getAnimations() { return animations; }

private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;
};