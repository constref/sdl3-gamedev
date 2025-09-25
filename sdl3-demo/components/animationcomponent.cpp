#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../commands.h"

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
		currentFrameChanged.notify(frameNumber);
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= 0 && index < animations.size());
	currentAnimation = index;
}

void AnimationComponent::onAttached()
{
	owner.getCommandDispatch().registerCommand(Commands::SetAnimation, this);
}

void AnimationComponent::onCommand(const Command &command)
{
	if (command.id == Commands::SetAnimation)
	{
		setAnimation(command.param.asInt);
	}
}
