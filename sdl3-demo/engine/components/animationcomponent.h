#pragma once

#include <vector>
#include "component.h"

class Animation;
class SetAnimationDPump;

class AnimationComponent : public Component
{
public:
	static const int NO_ANIMATION = -1;

	AnimationComponent(GameObject &owner, const std::vector<Animation> &animation);
	void update(const FrameContext &ctx) override;
	void setAnimation(int index);
	void onAttached(DataDispatcher &dataDispatcher) override;

	void onData(const SetAnimationDPump &dp);
private:
	int currentAnimation = NO_ANIMATION;
	std::vector<Animation> animations;
	int frameNumber;

};