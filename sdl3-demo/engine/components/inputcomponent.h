#pragma once

#include <components/component.h>

class InputComponent : public Component
{
	float direction;

public:
	InputComponent(GameObject &owner);
	void update(const FrameContext &ctx) override;
};