#pragma once

#include <components/component.h>

class CollisionEvent;

class ProjectileComponent : public Component
{
public:
	ProjectileComponent(Node &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onEvent(const CollisionEvent &event);
};