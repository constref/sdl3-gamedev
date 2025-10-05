#pragma once

class GameObject;

class State
{
public:
	virtual void onEnter(GameObject &owner) = 0;
	virtual void onExit(GameObject &owner) = 0;
	virtual void onEvent(GameObject &owner, int eventId) = 0;
};

