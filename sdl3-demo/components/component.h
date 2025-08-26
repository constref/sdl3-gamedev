#pragma once

struct GameObject;
struct FrameContext;

class Component
{
protected:
	GameObject &owner;

public:
	Component(GameObject &owner) : owner(owner) {}
	virtual ~Component() {}
	virtual void update(const FrameContext &ctx) = 0;
};
