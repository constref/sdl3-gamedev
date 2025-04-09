#pragma once

class Timer
{
	float length, time;
public:
	Timer(float length) : length(length), time(0)
	{
	}

	// return true one timeout
	bool step(float deltaTime)
	{
		time += deltaTime;
		if (time >= length)
		{
			time -= length;
			return true;
		}
		return false;
	}

	void reset() { time = 0; }
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
	Animation(int frameCount, float length)
		: frameCount(frameCount), loopCount(0), timer(length)
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