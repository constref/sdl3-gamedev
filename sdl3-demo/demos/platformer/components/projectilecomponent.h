#pragma once

#include <components/component.h>

class CollisionEvent;
class NodeRemovalEvent;
class AnimationEndEvent;

class ProjectileComponent : public Component
{
	int collisions;
public:
	ProjectileComponent(Node &owner);

	void onEvent(const CollisionEvent &event);
	void onEvent(const NodeRemovalEvent &event);
	void onEvent(const AnimationEndEvent &event);
};