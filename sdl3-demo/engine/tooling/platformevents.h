#pragma once
#include <variant>

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

using PlatformEvent = std::variant<
	ApplicationEnteredBackground,
	ApplicationEnteredForeground,
	MouseMoveEvent,
	MouseButtonEvent,
	ResizeEvent,
	ExitEvent
>;
