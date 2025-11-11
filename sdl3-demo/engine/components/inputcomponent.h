#pragma once

#include <components/component.h>
#include <nodehandle.h>

class InputComponent : public Component
{
	NodeHandle ownerHandle;

public:
	InputComponent(Node &owner, NodeHandle ownerHandle);
};