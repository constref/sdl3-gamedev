#pragma once

#include <systems/system.h>
#include <components/spritecomponent.h>
#include <components/animationcomponent.h>

class AnimationPlayEvent;
class DirectionChangedEvent;

class SpriteRenderSystem : public System<FrameStage::Render, SpriteComponent>
{
public:
	SpriteRenderSystem(Services &services);
	void update(Node &node) override;
	void onEvent(NodeHandle target, const AnimationPlayEvent &event);
	void onEvent(NodeHandle target, const DirectionChangedEvent &event);
};