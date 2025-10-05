#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../messaging/messages.h"
#include "../messaging/coresubjects.h"

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
		currentFrameSubject.notify(frameNumber);
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= 0 && index < animations.size());
	currentAnimation = index;
}

void AnimationComponent::onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<AnimationComponent, SetAnimationMessage>(this);
	registry.registerSubject(CoreSubjects::CURRENT_ANIMATION_FRAME, &currentFrameSubject);
}

void AnimationComponent::onMessage(const SetAnimationMessage &msg)
{
	setAnimation(msg.getAnimationIndex());
	animations[currentAnimation].reset();
}
