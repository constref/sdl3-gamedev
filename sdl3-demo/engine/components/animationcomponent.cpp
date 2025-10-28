#include "animationcomponent.h"
#include "../animation.h"
#include "../node.h"
#include "../framecontext.h"
#include "../messaging/commands.h"
#include <messaging/events.h>
#include <messaging/eventqueue.h>

#include <cassert>

AnimationComponent::AnimationComponent(Node &owner, const std::vector<Animation> &animations) : Component(owner, ComponentStage::Animation)
{
	this->animations = animations;
	this->frameNumber = 1;
}

void AnimationComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	dataDispatcher.registerHandler<AnimationComponent, SetAnimationCommand>(this);
}

void AnimationComponent::update(const FrameContext &ctx)
{
	if (currentAnimation != NO_ANIMATION)
	{
		if (animations[currentAnimation].step(ctx.deltaTime))
		{
			EventQueue::get().enqueue<AnimationEndEvent>(owner.getHandle(), ComponentStage::Animation, currentAnimation);
		}
		frameNumber = animations[currentAnimation].currentFrame() + 1;
		owner.pushData(FrameChangeCommand{ frameNumber });
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
}
