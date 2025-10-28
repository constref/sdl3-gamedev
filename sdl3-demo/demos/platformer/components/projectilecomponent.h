#pragma once

#include <components/component.h>

class CollisionEvent;
class NodeRemovalEvent;

class ProjectileComponent : public Component
{
	int collisions;
public:
	ProjectileComponent(Node &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onEvent(const CollisionEvent &event);
	void onEvent(const NodeRemovalEvent &event);
};