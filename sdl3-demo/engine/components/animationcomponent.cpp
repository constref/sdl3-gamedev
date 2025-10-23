#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../messaging/datapumps.h"

#include <cassert>

AnimationComponent::AnimationComponent(GameObject &owner, const std::vector<Animation> &animations) : Component(owner, ComponentStage::Animation)
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
		owner.sendMessage(FrameChangeDPump{ frameNumber });
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= -1 && index < (int)animations.size());
	currentAnimation = index;
}

void AnimationComponent::onAttached(DataDispatcher &dataDispatcher)
{
	dataDispatcher.registerHandler<AnimationComponent, SetAnimationDPump>(this);
}

void AnimationComponent::onData(const SetAnimationDPump &dp)
{
	setAnimation(dp.getAnimationIndex());
	animations[currentAnimation].reset();
}
