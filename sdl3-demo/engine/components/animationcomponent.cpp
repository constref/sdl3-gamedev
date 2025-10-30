#include "animationcomponent.h"

#include <messaging/messaging.h>
#include "../animation.h"
#include "../node.h"
#include "../framecontext.h"
#include <cassert>

AnimationComponent::AnimationComponent(Node &owner, const std::vector<Animation> &animations) : Component(owner, ComponentStage::Animation)
{
	this->animations = animations;
	this->frameNumber = 1;
	this->notifyEnd = false;
	this->playing = true;
	owner.getCommandDispatcher().registerHandler<SetAnimationCommand>(this);
	owner.getEventDispatcher().registerHandler<AnimationStopEvent>(this);
}

void AnimationComponent::update(const FrameContext &ctx)
{
	if (currentAnimation != NO_ANIMATION && playing)
	{
		int timeouts = animations[currentAnimation].step(ctx.deltaTime);
		if (timeouts && notifyEnd)
		{
			EventQueue::get().enqueue<AnimationEndEvent>(owner.getHandle(), ComponentStage::Animation, currentAnimation);
			notifyEnd = false;
		}
		else
		{
			frameNumber = animations[currentAnimation].currentFrame() + 1;
			owner.sendCommand(FrameChangeCommand{ frameNumber });
		}
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= -1 && index < (int)animations.size());
	currentAnimation = index;
}

void AnimationComponent::onCommand(const SetAnimationCommand &dp)
{
	setAnimation(dp.getAnimationIndex());
	animations[currentAnimation].reset();
	notifyEnd = dp.shouldNotifyEnd();
}

void AnimationComponent::onEvent(const AnimationStopEvent &event)
{
	playing = false;
}
