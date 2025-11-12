#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>

#include "../components/playercontrollercomponent.h"

class CollisionEvent;
class FallingEvent;
class JumpEvent;
class ShootBeginEvent;
class ShootEndEvent;

class PlayerControlSystem : public System<FrameStage::Gameplay, InputComponent, PlayerControllerComponent, PhysicsComponent>
{
public:
	PlayerControlSystem();
	void update(Node &node) override;
	void transitionState(Node &node, PState newState);

	void onEvent(NodeHandle target, const CollisionEvent &event);
	void onEvent(NodeHandle target, const FallingEvent &event);
	void onEvent(NodeHandle target, const JumpEvent &event);
	void onEvent(NodeHandle target, const ShootBeginEvent &event);
	void onEvent(NodeHandle target, const ShootEndEvent &event);
};