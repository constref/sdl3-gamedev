#pragma once
#include <SDL3/SDL.h>
#include <deque>
#include <cassert>
#include <nodehandle.h>

struct KeyEvent
{
	SDL_Scancode scancode;
	bool pressed;
};

class InputState
{
	bool keys[SDL_SCANCODE_COUNT]{ false };
	std::deque<KeyEvent> keyEvents;
	NodeHandle focusTarget;

public:
	void setKeyState(SDL_Scancode scancode, bool pressed)
	{
		assert(scancode >= 0 && scancode < SDL_SCANCODE_COUNT);
		keys[scancode] = pressed;
	}
	bool isKeyPressed(SDL_Scancode scancode) const
	{
		assert(scancode >= 0 && scancode < SDL_SCANCODE_COUNT);
		return keys[scancode];
	}
	void addEvent(SDL_Scancode scancode, bool pressed)
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