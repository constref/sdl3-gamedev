#pragma once

#include <systems/system.h>
#include <components/spritecomponent.h>
#include <components/animationcomponent.h>

class SpriteRenderSystem : public System<FrameStage::Render, SpriteComponent>
{
public:
	void update(Node &node) override;
};