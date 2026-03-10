#pragma once

#include <framestage.h>
#include <timer.h>
#include <vector>
#include <typeindex>

static size_t nextId = 0;

class Node;
class CommandDispatcher;
class EventDispatcher;
class TimerTimeoutEvent;

using ComponentId = unsigned long;

class Component
{
	std::vector<Timer *> timers;
	Node &m_owner;

public:
	Component(Node &owner) : m_owner(owner) { }
	Component(Component &) = delete;
	Component(Component &&) = delete;
	virtual ~Component();
	
	void operator=(const Component &) = delete;
	
	virtual void onStart() {}
	virtual void earlyUpdate();
	virtual void update() {}

	Node &owner() { return m_owner; }
	void addTimer(Timer &timer);
	void removeTimer(const Timer &timer);


	template<typename ComponentType = Component>
	constexpr static size_t typeId()
	{
		// returns a size_t casted address of the static typeId var for this specialization
		static size_t typeId = reinterpret_cast<size_t>(&typeId);
		//static size_t typeId = ++nextId;
		return typeId;
	}
};