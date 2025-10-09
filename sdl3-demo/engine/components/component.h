#pragma once

class GameObject;
struct FrameContext;
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
	virtual void onEvent(int eventId) {}
	virtual void update(const FrameContext &ctx) = 0;
	void emit(const FrameContext &ctx, int eventId);
};
