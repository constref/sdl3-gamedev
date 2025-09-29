#pragma once

#include "../component.h"

enum class PState
{
	idle,
	running,
	airborne,
	falling,
	sliding
};

class StateComponent : public Component
{
	PState currentState;

public:
	StateComponent(GameObject &owner);
	~StateComponent() override {}

	void transitionState(PState newState);
	void update(const FrameContext &ctx) override;
	void onEvent(int eventId) override;
};