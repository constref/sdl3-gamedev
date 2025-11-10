#pragma once

#include <framestage.h>
#include <timer.h>
#include <vector>

class Node;
class CommandDispatcher;
class EventDispatcher;
class TimerOnTimeout;

using ComponentId = unsigned long;

class Component
{
	static inline ComponentId nextId;
	ComponentId id;
	FrameStage stage;
	std::vector<Timer *> timers;

protected:
	Node &owner;

public:
	Component(Node &owner, FrameStage stage) : owner(owner), stage(stage) { id = ++nextId; }
	FrameStage getStage() const { return stage; }
	virtual ~Component();

	ComponentId getId() const { return id; }
	virtual void onStart() {}
	virtual void earlyUpdate();
	virtual void update() {}

	void addTimer(Timer &timer);
	void removeTimer(const Timer &timer);
};
