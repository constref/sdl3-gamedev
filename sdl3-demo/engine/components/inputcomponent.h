#pragma once

#include <components/component.h>
#include <ghandle.h>

struct KeyboardEvent;

class InputComponent : public Component
{
	float direction;
	GHandle ownerHandle;

public:
	InputComponent(GameObject &owner, GHandle ownerHandle);
	void update(const FrameContext &ctx) override;
	void onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onEvent(const KeyboardEvent &event);
};