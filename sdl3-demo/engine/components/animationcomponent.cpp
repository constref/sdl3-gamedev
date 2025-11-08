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

	owner.getEventDispatcher().registerHandler<AnimationPlayEvent>(this);
	owner.getEventDispatcher().registerHandler<AnimationStopEvent>(this);
}

void AnimationComponent::update()
{
	if (currentAnimation != NO_ANIMATION)
	{
		// check if animation has ended
		int timeouts = animations[currentAnimation].step(FrameContext::global().deltaTime);
		if (!timeouts)
		{
			// if not, get frameNumber as usual
			frameNumber = animations[currentAnimation].currentFrame() + 1;
			owner.sendCommand(FrameChangeCommand{ frameNumber });
		}
		else
		{
			if (playbackMode == AnimationPlaybackMode::oneShot) // one-shot animation, remove the current animation
			{
				currentAnimation = NO_ANIMATION;
			}
			else // continuous play, send out updated frameNumber (wrapped-around back to 0)
			{
				frameNumber = animations[currentAnimation].currentFrame() + 1;
				owner.sendCommand(FrameChangeCommand{ frameNumber });
			}
		}
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= -1 && index < (int)animations.size());
	currentAnimation = index;
}

void AnimationComponent::onEvent(const AnimationStopEvent &event)
{
}

void AnimationComponent::onEvent(const AnimationPlayEvent &event)
{
	setAnimation(event.getAnimationIndex());
	playbackMode = event.getPlaybackMode();
}
