#pragma once

#include <vector>
#include "component.h"

class Animation;
class SetAnimationCommand;
class AnimationStopEvent;

class AnimationComponent : public Component
{
	bool notifyEnd;
	bool playing;

public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(Node &owner, const std::vector<Animation> &animation);
	void update(const FrameContext &ctx) override;
	void setAnimation(int index);
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onCommand(const SetAnimationCommand &dp);
	void onEvent(const AnimationStopEvent &event);
private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;
};