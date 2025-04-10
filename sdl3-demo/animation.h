#pragma once

class Timer
{
	float length, time;
	bool timeout, oneshot;
public:
	Timer(float length, bool oneshot = false) : length(length), time(0), timeout(false), oneshot(oneshot)
	{
	}

	// return true one timeout
	bool step(float deltaTime)
	{
		float prevTime = time;
		time += deltaTime;
		if (time >= length)
		{
			if (!oneshot)
			{
				time -= length;
			}
			else
			{
				time = prevTime;
			}
			timeout = true;
			return true;
		}
		return false;
	}

	void reset() { time = 0; timeout = false; }
	bool isTimedOut() const { return timeout; }
	void setTime(float time) { this->time = time; }
	float currentTime() const { return time; }
	float getLength() const { return length; }
};

class Animation
{
	int frameCount, loopCount;
	Timer timer;

public:
	Animation() : frameCount(0), loopCount(0), timer(0)
	{
	}
	Animation(int frameCount, float length, bool oneshot = false)
		: frameCount(frameCount), loopCount(0), timer(length, oneshot)
	{
	}

	void progress(float deltaTime)
	{
		if (timer.step(deltaTime))
		{
			loopCount++;
		}
	}

	void reset() { timer.reset(); }
	int currentFrame() const
	{
		return static_cast<int>((timer.currentTime() / timer.getLength()) * frameCount);
	}

	int getFrameCount() const { return frameCount; }
	int getLoopCount() const { return loopCount; }
};