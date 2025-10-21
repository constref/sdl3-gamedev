#pragma once

#include <vector>
#include "component.h"

class Animation;
class SetAnimationMessage;

class AnimationComponent : public Component
{
public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(GameObject &owner, const std::vector<Animation> &animation);
	void update(const FrameContext &ctx) override;
	void setAnimation(int index);
	void onAttached(MessageDispatch &msgDispatch) override;

	void onMessage(const SetAnimationMessage &msg);
private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;

};