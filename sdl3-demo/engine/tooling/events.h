#pragma once

#include <messaging/event.h>

class MouseRelativeModeToggleEvent : public Event<MouseRelativeModeToggleEvent, FrameStage::Input>
{
};