#pragma once

#include <vector>
#include "component.h"

class Animation;
class SetAnimationCommand;

class AnimationComponent : public Component
{
	bool notifyEnd;

public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(Node &owner, const std::vector<Animation> &animation);
	void update(const FrameContext &ctx) override;
	void setAnimation(int index);
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onCommand(const SetAnimationCommand &dp);
private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;

};