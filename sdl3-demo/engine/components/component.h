#pragma once

struct FrameContext;
class GameObject;
class SubjectRegistry;
class MessageDispatch;

class Component
{
protected:
	GameObject &owner;

public:
	Component(GameObject &owner) : owner(owner) {}
	virtual ~Component() {}
	virtual void onAttached(MessageDispatch &dispatch) {}
	virtual void onStart() {}
	virtual void update(const FrameContext &ctx) = 0;
	void emit(const FrameContext &ctx, int eventId);
};
