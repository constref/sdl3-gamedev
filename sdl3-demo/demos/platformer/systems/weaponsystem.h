#pragma once

#include <systems/system.h>

#include "../components/weaponcomponent.h"
#include <components/physicscomponent.h>

class ShootBeginEvent;
class ShootEndEvent;
class CollisionEvent;
class NodeRemovalEvent;
//class TimerOnTimeout;

class WeaponSystem : public System<FrameStage::Gameplay, WeaponComponent, PhysicsComponent>
{
	glm::vec2 fireDirection;
public:
	WeaponSystem(Services &services);
	void update(Node &node);

	void onEvent(NodeHandle target, const ShootBeginEvent &event);
	void onEvent(NodeHandle target, const ShootEndEvent &event);
	void onEvent(NodeHandle target, const CollisionEvent &event);
	void onEvent(NodeHandle target, const NodeRemovalEvent &event);
	//void onEvent(NodeHandle target, const TimerOnTimeout &event);
};