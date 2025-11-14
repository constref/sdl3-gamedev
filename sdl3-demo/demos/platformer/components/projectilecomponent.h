#pragma once

#include <components/component.h>

class UpdateViewportCommand;
class CollisionEvent;
class NodeRemovalEvent;

class ProjectileComponent : public Component
{
	int collisions;

public:
	ProjectileComponent(Node &owner);

	void onEvent(const CollisionEvent &event);
	void onEvent(const NodeRemovalEvent &event);
};