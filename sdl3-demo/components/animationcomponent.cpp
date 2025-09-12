#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"
#include "../framecontext.h"

#include <cassert>

AnimationComponent::AnimationComponent(const std::vector<Animation> &animations) : Component()
{
	this->animations = animations;
	this->frameNumber = 1;
}

void AnimationComponent::update(GameObject &owner, const FrameContext &ctx)
{
	if (currentAnimation != NO_ANIMATION)
	{
		animations[currentAnimation].step(ctx.deltaTime);
		frameNumber = animations[currentAnimation].currentFrame() + 1;
		currentFrameChanged.notify(frameNumber);
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= 0 && index < animations.size());
	currentAnimation = index;
}
