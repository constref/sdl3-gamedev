#pragma once
#include <deque>
#include <cassert>
#include <nodehandle.h>
#define SCANCODE_COUNT 512

using Scancode = uint16_t;

struct KeyEvent
{
	Scancode scancode;
	bool pressed;
};

class InputState
{
	bool keys[SCANCODE_COUNT]{ false };
	std::deque<KeyEvent> keyEvents;
	NodeHandle focusTarget;

public:
	void setKeyState(Scancode scancode, bool pressed)
	{
		assert(scancode >= 0 && scancode < SCANCODE_COUNT);
		keys[scancode] = pressed;
	}
	bool isKeyPressed(uint16_t scancode) const
	{
		assert(scancode >= 0 && scancode < SCANCODE_COUNT);
		return keys[scancode];
	}
	void addEvent(Scancode scancode, bool pressed)
	{
		keyEvents.push_back(KeyEvent{ scancode, pressed });
	}
	bool popEvent(KeyEvent &event)
	{
		if (keyEvents.empty())
		{
			return false;
		}
		event = keyEvents.front();
		keyEvents.pop_front();
		return true;
	}

	NodeHandle getFocusTarget() const { return focusTarget; }
	void setFocus(NodeHandle focusTarget)
	{
		this->focusTarget = focusTarget;
	}
};