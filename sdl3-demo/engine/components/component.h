#pragma once

#include <components/componentstage.h>

struct FrameContext;
class GameObject;
class SubjectRegistry;
class CommandDispatcher;
class EventDispatcher;

class Component
{
	ComponentStage stage;

protected:
	GameObject &owner;

public:
	Component(GameObject &owner, ComponentStage stage) : owner(owner), stage(stage) {}
	ComponentStage getStage() const { return stage; }
	virtual ~Component() {}
	virtual void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) {}
	virtual void onStart() {}
	virtual void update(const FrameContext &ctx) = 0;
	void emit(const FrameContext &ctx, int eventId);
};
