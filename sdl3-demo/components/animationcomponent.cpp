#include "animationcomponent.h"
#include "../animation.h"
#include "../gameobject.h"

#include <cassert>

AnimationComponent::AnimationComponent(const std::vector<Animation> &animations, GameObject &owner) : Component(owner)
{
	this->animations = animations;
}

void AnimationComponent::update(SDLState &state, GameState &gs, Resources &res, float deltaTime)
{
	if (currentAnimation != NO_ANIMATION)
	{
		animations[currentAnimation].step(deltaTime);
		owner.spriteFrame = animations[currentAnimation].currentFrame() + 1;
	}
}

void AnimationComponent::setAnimation(int index)
{
	assert(index >= 0 && index < animations.size());
	currentAnimation = index;
}
