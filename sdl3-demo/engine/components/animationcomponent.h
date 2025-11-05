#pragma once

#include <vector>
#include "component.h"

class Animation;
class SetAnimationCommand;
class AnimationStopEvent;
class AnimationPlayEvent;

class AnimationComponent : public Component
{
	bool notifyEnd;
	bool playing;

public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(Node &owner, const std::vector<Animation> &animation);
	void update() override;
	void setAnimation(int index);
	void onCommand(const SetAnimationCommand &dp);
	void onEvent(const AnimationStopEvent &event);
	void onEvent(const AnimationPlayEvent &event);

private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;
};