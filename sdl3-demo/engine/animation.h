#pragma once
#include "timer.h"

class Animation
{
	Timer timer;
	int frameCount;

public:
	Animation() : timer(0), frameCount(0) {}
	Animation(int frameCount, float length) : frameCount(frameCount), timer(length)
	{
	}

	float getLength() const { return timer.getLength(); }
	int currentFrame() const
	{
		return static_cast<int>(timer.getTime() / timer.getLength() * frameCount);
	}

	void step(float deltaTime)
	{
		timer.step(deltaTime);
	}

	void reset()
	{
		timer.reset();
	}

	bool isDone() const { return timer.getTimeouts() > 0; }
};
