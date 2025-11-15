#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>

class KeyboardEvent;

class InputSystem : public System<FrameStage::Input, InputComponent>
{
public:
	InputSystem(Services &services);
	void update(Node &node) override;
	void onEvent(NodeHandle hNode, const KeyboardEvent &event);
	void onLinked(Node &node) override;
};