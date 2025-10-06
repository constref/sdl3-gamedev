#pragma once

#include <components/component.h>
#include <messaging/observer.h>

class InputComponent : public Component
{
	float direction;

public:
	InputComponent(GameObject &owner);
	void update(const FrameContext &ctx) override;
	void onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch) override;

	Subject<float> directionSubject;
};