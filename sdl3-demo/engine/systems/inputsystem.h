#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>

class KeyUpEvent;
class KeyDownEvent;
class MouseMotionEvent;

class InputSystem : public System<FrameStage::Input, InputComponent>
{
public:
	InputSystem(Services &services);
	void update(Node &node) override;
	void onEvent(NodeHandle hNode, const KeyUpEvent &event);
	void onEvent(NodeHandle hNode, const KeyDownEvent &event);
	void onEvent(NodeHandle hNode, const MouseMotionEvent &event);
	void onLinked(Node &node) override;

	void handleDirectionChange(NodeHandle target, const InputState &state, InputComponent &inputComp) const;
};