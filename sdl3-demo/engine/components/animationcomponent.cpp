#include "animationcomponent.h"

#include <messaging/messaging.h>
#include <animation.h>
#include <node.h>
#include <framecontext.h>
#include <cassert>

AnimationComponent::AnimationComponent(Node &owner) : Component(owner)
{
	this->frameNumber = 1;
	this->playbackMode = AnimationPlaybackMode::continuous;
}

void AnimationComponent::update()
{
}

ResourceId AnimationComponent::getAnimationId() const
{
	return currentAnimation;
}

void AnimationComponent::setAnimation(ResourceId animId)
{
	currentAnimation = animId;
}
