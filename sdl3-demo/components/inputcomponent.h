#pragma once

#include "component.h"
#include "observer.h"

class InputComponent : public Component
{
public:
	InputComponent();
	void update(GameObject &owner, const FrameContext &ctx) override;

	Subject<float> directionUpdate;
};