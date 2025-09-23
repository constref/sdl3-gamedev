#pragma once

#include <memory>

struct GameObject;
struct FrameContext;

class Component
{
protected:
	std::weak_ptr<GameObject> owner;

public:
	Component(std::shared_ptr<GameObject> owner) : owner(owner) {}
	virtual ~Component() {}
	virtual void update(const FrameContext &ctx) = 0;

	void emit(int eventId);
	virtual void eventHandler(int eventId) {}
};
