#pragma once

#include "../state.h"

class SlidingState : public State
{
public:
	static State *instance()
	{
		static RunningState instance;
		return &instance;
	}

	void onEnter(GameObject &owner) override;
	void onExit(GameObject &owner) override;
};