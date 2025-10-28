#pragma once

#include <components/componentstage.h>

struct FrameContext;
class Node;
class SubjectRegistry;
class CommandDispatcher;
class EventDispatcher;

class Component
{
	ComponentStage stage;

protected:
	Node &owner;

public:
	Component(Node &owner, ComponentStage stage) : owner(owner), stage(stage) {}
	ComponentStage getStage() const { return stage; }
	virtual ~Component() {}
	virtual void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) {}
	virtual void onStart() {}
	virtual void update(const FrameContext &ctx) {}
	void emit(const FrameContext &ctx, int eventId);
};
