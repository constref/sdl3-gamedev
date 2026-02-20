#pragma once
#include <variant>
#include <stdint.h>

struct MouseMoveEvent
{
    int x;
	int y;
};

struct MouseButtonEvent
{
	int buttonIndex;
	bool isDown;
};

struct KeyUp
{
	uint16_t scancode;
};
struct KeyDown
{
	uint16_t scancode;
};

struct ResizeEvent
{
	int x;
	int y;
	int width;
	int height;
	ResizeEvent(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
};

struct ExitEvent
{
};

struct ApplicationEnteredBackground
{
};

struct ApplicationEnteredForeground
{
};

struct InteropMessage
{
	uint32_t messageType;
};

using PlatformEvent = std::variant<
	ApplicationEnteredBackground,
	ApplicationEnteredForeground,
	KeyUp,
	KeyDown,
	MouseMoveEvent,
	MouseButtonEvent,
	ResizeEvent,
	ExitEvent,
	InteropMessage
>;
