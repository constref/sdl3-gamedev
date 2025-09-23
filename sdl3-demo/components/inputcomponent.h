#pragma once

#include "component.h"
#include "observer.h"

class InputComponent : public Component
{
	float direction;

public:
	InputComponent(std::shared_ptr<GameObject> owner);
	void update(const FrameContext &ctx) override;

	Subject<float> directionUpdate;
};