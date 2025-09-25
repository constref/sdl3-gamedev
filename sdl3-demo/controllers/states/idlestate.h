#pragma once

#include "../../state.h"

class IdleState : public State
{
public:
	static State *instance()
	{
		static IdleState instance;
		return &instance;
	}

	void onEnter(GameObject &owner) override;
	void onExit(GameObject &owner) override;
	void onEvent(GameObject &owner, int eventId) override;
};

