#pragma once

#include <vector>
#include <animationplaybackmode.h>
#include <resourceid.h>
#include "component.h"

class Animation;

class AnimationComponent : public Component
{
	AnimationPlaybackMode playbackMode;
	ResourceId currentAnimation;
	float time;

public:
	AnimationComponent(Node &owner);
	void update() override;
	ResourceId getAnimationId() const;
	void setAnimation(ResourceId animId);
	AnimationPlaybackMode getPlaybackMode() const { return playbackMode; }
	void setPlaybackMode(AnimationPlaybackMode mode)
	{
		playbackMode = mode;
	}
	int getFrameNumber() const { return frameNumber; }
	float getTime() const { return time; }
	void setTime(float time) { this->time = time; }

private:
	int frameNumber;
};