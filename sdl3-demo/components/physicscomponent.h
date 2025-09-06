#pragma once

#include  "component.h"

struct FrameContext;
class InputComponent;

class PhysicsComponent : public Component
{
	float direction;

public:
	PhysicsComponent(InputComponent *inputComponent, GameObject &owner);
	void update(const FrameContext &ctx);
};
