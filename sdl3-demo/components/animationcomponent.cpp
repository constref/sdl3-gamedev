#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"
#include "../framecontext.h"
#include "../commands.h"
#include "../coresubjects.h"

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

void AnimationComponent::onAttached(SubjectRegistry &registry)
{
	owner.getCommandDispatch().registerHandler<SetAnimationCommand>(this);
	//owner.getCommandDispatch().registerCommand(Commands::SetAnimation, this);
	registry.registerSubject(CoreSubjects::CURRENT_ANIMATION_FRAME, &currentFrameSubject);
}

void AnimationComponent::onCommand(const SetAnimationCommand &cmd)
{
	printf("AnimationComponent::onCommand SetAnimationCommand %d\n", cmd.getAnimationIndex());
	//if (command.id == Commands::SetAnimation)
	//{
	//	if (currentAnimation != command.param.asInt)
	//	{
	//		setAnimation(command.param.asInt);
	//		animations[currentAnimation].reset();
	//	}
	//}
}
