#pragma once

#include <components/componentstage.h>

struct FrameContext;
class Node;
class SubjectRegistry;
class CommandDispatcher;
class EventDispatcher;

using ComponentId = unsigned long;

class Component
{
	static inline ComponentId nextId;
	ComponentId id;
	ComponentStage stage;

protected:
	Node &owner;

public:
	Component(Node &owner, ComponentStage stage) : owner(owner), stage(stage) { id = ++nextId; }
	ComponentStage getStage() const { return stage; }
	virtual ~Component();

	ComponentId getId() const { return id; }
	virtual void onStart() {}
	virtual void update(const FrameContext &ctx) {}
};
