#pragma once

#include <components/component.h>
#include <nodehandle.h>

struct KeyboardEvent;

class InputComponent : public Component
{
	float direction;
	NodeHandle ownerHandle;

public:
	InputComponent(Node &owner, NodeHandle ownerHandle);
	void update(const FrameContext &ctx) override;
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onEvent(const KeyboardEvent &event);
};