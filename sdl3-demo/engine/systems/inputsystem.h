#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>

class KeyUpEvent;
class KeyDownEvent;

class InputSystem : public System<FrameStage::Input, InputComponent>
{
public:
	InputSystem(Services &services);
	void update(Node &node) override;
	void onEvent(NodeHandle hNode, const KeyUpEvent &event);
	void onEvent(NodeHandle hNode, const KeyDownEvent &event);
	void onLinked(Node &node) override;

	void handleDirectionChange(NodeHandle target, InputState &state, InputComponent &inputComp);
};