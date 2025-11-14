#pragma once

#include <systems/system.h>

#include "../components/weaponcomponent.h"
#include <components/physicscomponent.h>

class DirectionChangedEvent;
class ShootBeginEvent;
class ShootEndEvent;
class TimerOnTimeout;

class WeaponSystem : public System<FrameStage::Gameplay, WeaponComponent, PhysicsComponent>
{
	glm::vec2 playerDirection;

public:
	WeaponSystem(Services &services);
	void update(Node &node);

	void onEvent(NodeHandle target, const DirectionChangedEvent &event);
	void onEvent(NodeHandle target, const ShootBeginEvent &event);
	void onEvent(NodeHandle target, const ShootEndEvent &event);
	//void onEvent(NodeHandle target, const TimerOnTimeout &event);
};