#include <systems/spriteanimationsystem.h>
#include <components/animationcomponent.h>
#include <animation.h>
#include <framecontext.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <world.h>

SpriteAnimationSystem::SpriteAnimationSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<AnimationPlayEvent>(this);
	services.eventQueue().dispatcher.registerHandler<AnimationStopEvent>(this);
}

void SpriteAnimationSystem::update(Node &node)
{
	auto [ac, sc] = getRequiredComponents(node);

	int currentAnimation = ac->getAnimation();
	if (currentAnimation != AnimationComponent::NO_ANIMATION)
	{
		auto &animations = ac->getAnimations();
		// check if animation has ended
		int timeouts = animations[currentAnimation].step(FrameContext::dt());
		if (!timeouts)
		{
			// if not, get frameNumber as usual
			sc->setFrameNumber(animations[currentAnimation].currentFrame() + 1);
		}
		else
		{
			if (ac->getPlaybackMode() == AnimationPlaybackMode::oneShot) // one-shot animation, remove the current animation
			{
				currentAnimation = AnimationComponent::NO_ANIMATION;
			}
			else // continuous play, send out updated frameNumber (wrapped-around back to 0)
			{
				sc->setFrameNumber(animations[currentAnimation].currentFrame() + 1);
			}
		}
	}
}

void SpriteAnimationSystem::onEvent(NodeHandle target, const AnimationStopEvent &event)
{
}

void SpriteAnimationSystem::onEvent(NodeHandle target, const AnimationPlayEvent &event)
{
	Node &node = services.world().getNode(target);
	auto [ac, sc] = getRequiredComponents(node);

	ac->setAnimation(event.getAnimationIndex());
	ac->setPlaybackMode(event.getPlaybackMode());
}
