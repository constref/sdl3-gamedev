#pragma once

struct GameObject;

struct SDLState;
struct GameState;
struct Resources;

class Component
{
protected:
	GameObject &owner;

public:
	Component(GameObject &owner) : owner(owner) {}
	virtual ~Component() {}
	virtual void update(SDLState &state, GameState &gs, Resources &res, float deltaTime) = 0;
};
