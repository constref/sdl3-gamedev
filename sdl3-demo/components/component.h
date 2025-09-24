#pragma once

#include <functional>

class GameObject;
struct FrameContext;

class Component
{
	using EventHandler = std::function<void(int)>;
	EventHandler eventHandler;

protected:
	GameObject &owner;

public:
	Component(GameObject &owner) : owner(owner) {}
	virtual ~Component() {}
	virtual void update(const FrameContext &ctx) = 0;

	void emit(int eventId);
	virtual void eventHandler2(int eventId) {}

	EventHandler getEventHandler() const { return eventHandler; }
	void setEventHandler(EventHandler handler) { this->eventHandler = handler; }
};
