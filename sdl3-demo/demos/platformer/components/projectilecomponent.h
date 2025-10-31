#pragma once

#include <components/component.h>

class UpdateViewportCommand;
class CollisionEvent;
class NodeRemovalEvent;
class AnimationEndEvent;

class ProjectileComponent : public Component
{
	int collisions;

public:
	ProjectileComponent(Node &owner);

	void onCommand(const UpdateViewportCommand &dp);
	void onEvent(const CollisionEvent &event);
	void onEvent(const NodeRemovalEvent &event);
	void onEvent(const AnimationEndEvent &event);
};