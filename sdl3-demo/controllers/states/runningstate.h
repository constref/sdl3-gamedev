#pragma once

#include "../../state.h"

class RunningState : public State
{
public:
	static State *instance()
	{
		static RunningState instance;
		return &instance;
	}

	void onEnter(GameObject &owner) override;
	void onExit(GameObject &owner) override;
	void onEvent(GameObject &owner, int eventId) override;
};
