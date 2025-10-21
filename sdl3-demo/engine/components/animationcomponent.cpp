#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../messaging/messages.h"

#include <cassert>

AnimationComponent::AnimationComponent(GameObject &owner, const std::vector<Animation> &animations) : Component(owner)
{
	this->animations = animations;
	this->frameNumber = 1;
}

void AnimationComponent::update(const FrameContext &ctx)
{
	if (currentAnimation != NO_ANIMATION)
	{
		animations[currentAnimation].step(ctx.deltaTime);
		frameNumber = animations[currentAnimation].currentFrame() + 1;
		owner.sendMessage(FrameChangeMessage{ frameNumber });
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= -1 && index < (int)animations.size());
	currentAnimation = index;
}

void AnimationComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<AnimationComponent, SetAnimationMessage>(this);
}

void AnimationComponent::onMessage(const SetAnimationMessage &msg)
{
	setAnimation(msg.getAnimationIndex());
	animations[currentAnimation].reset();
}
