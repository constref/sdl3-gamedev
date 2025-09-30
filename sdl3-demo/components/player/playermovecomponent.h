#pragma once

#include "../component.h"

class PlayerMoveComponent : public Component
{
public:
	PlayerMoveComponent(GameObject &owner);
	~PlayerMoveComponent() override;
	void update(const FrameContext &ctx) override;
};