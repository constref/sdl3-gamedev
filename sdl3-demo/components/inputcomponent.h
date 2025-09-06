#pragma once

#include "component.h"
#include "observer.h"

class InputComponent : public Component
{
public:
	InputComponent(GameObject &owner);
	void update(const FrameContext &ctx) override;

	Subject<float> directionUpdate;
};