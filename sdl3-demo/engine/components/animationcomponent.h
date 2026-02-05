#pragma once

#include <vector>
#include <animationplaybackmode.h>
#include <resourceid.h>
#include <components/component.h>
#include <animation.h>

class AnimationComponent : public Component
{
	AnimationPlaybackMode playbackMode;
	ResourceId currentAnimation;

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

	Animation animation;

private:
	int frameNumber;
};