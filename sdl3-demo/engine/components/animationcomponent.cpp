#include "animationcomponent.h"

#include <messaging/messaging.h>
#include <animation.h>
#include <node.h>
#include <framecontext.h>
#include <cassert>

AnimationComponent::AnimationComponent(Node &owner, const std::vector<Animation> &animations) : Component(owner, FrameStage::Animation)
{
	this->animations = animations;
	this->frameNumber = 1;
	this->playbackMode = AnimationPlaybackMode::continuous;
}

void AnimationComponent::update()
{
}

int AnimationComponent::getAnimation() const
{
	return currentAnimation;
}
void AnimationComponent::setAnimation(int index)
{
	assert(index >= -1 && index < (int)animations.size());
	currentAnimation = index;
}
