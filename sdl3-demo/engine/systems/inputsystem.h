#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>

class KeyboardEvent;

class InputSystem : public System<InputComponent>
{
public:
	InputSystem();
	void update(Node &node) override;

	void onEvent(NodeHandle hNode, const KeyboardEvent &event);
};