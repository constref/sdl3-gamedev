#pragma once

#include <framestage.h>

struct FrameContext;
class Node;
class SubjectRegistry;
class CommandDispatcher;
class EventDispatcher;
class TimerOnTimeout;

using ComponentId = unsigned long;

class Component
{
	static inline ComponentId nextId;
	ComponentId id;
	FrameStage stage;

protected:
	Node &owner;

public:
	Component(Node &owner, FrameStage stage) : owner(owner), stage(stage) { id = ++nextId; }
	FrameStage getStage() const { return stage; }
	virtual ~Component();

	ComponentId getId() const { return id; }
	virtual void onStart() {}
	virtual void update() {}
};
