#pragma once

#include <math.h>

class Timer
{
	float length, time;
	int timeouts;

public:
	Timer(float length) : length(length), time(0), timeouts(0)
	{
	}

	int step(float deltaTime)
	{
		time += deltaTime;
		int timeouts = 0;
		if (time >= length)
		{
			// ensure large dt values don't cause problems
			// with very short timer lengths
			timeouts = static_cast<int>(time / length); // how many does dt cause timer to timeout
			this->timeouts += timeouts;
			time -= timeouts * length;
		}
		return timeouts;
	}

	int getTimeouts() const { return timeouts; }
	float getTime() const { return time; }
	float getLength() const { return length; }
	void reset() { time = 0, timeouts = 0; }
};