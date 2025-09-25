#pragma once

class GameObject;
struct FrameContext;
struct Command;

class Component
{
protected:
	GameObject &owner;

public:
	Component(GameObject &owner) : owner(owner) {}
	virtual ~Component() {}
	virtual void onAttached() {}
	virtual void onCommand(const Command &command) {}
	virtual void update(const FrameContext &ctx) = 0;
	void emit(const FrameContext &ctx, int eventId);
};
