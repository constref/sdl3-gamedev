#pragma once

#include "component.h"

class InputComponent : public Component
{
public:
	void update(const FrameContext &ctx) override;
};