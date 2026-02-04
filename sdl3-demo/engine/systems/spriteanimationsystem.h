#pragma once

#include <systems/system.h>
#include <components/animationcomponent.h>
#include <components/spritecomponent.h>
#include <animation.h>
#include <resourceid.h>

class AnimationStopEvent;
class AnimationPlayEvent;

class SpriteAnimationSystem : public System<FrameStage::Animation, AnimationComponent, SpriteComponent>
{
	std::vector<Animation> animations;

public:
	SpriteAnimationSystem(Services &services);
	void update(Node &node) override;

	void onEvent(NodeHandle target, const AnimationPlayEvent &event);
	void onEvent(NodeHandle target, const AnimationStopEvent &event);

	ResourceId createAnimation(int frameCount, float length);
};