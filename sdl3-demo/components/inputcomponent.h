#pragma once

#include "../component.h"
#include "../observer.h"

class InputComponent : public Component
{
	float direction;

public:
	InputComponent(GameObject &owner);
	void update(const FrameContext &ctx) override;
	void onAttached(SubjectRegistry &registry) override;

	Subject<float> directionSubject;
};