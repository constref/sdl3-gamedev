#pragma once

class Animation
{
	int frameCount, loopCount;
	float length, time;
public:
	Animation() : frameCount(0), length(1), time(0)
	{
	}
	Animation(int frameCount, float length)
		: frameCount(frameCount), length(length), time(0)
	{
	}

	void progress(float deltaTime)
	{
		time += deltaTime;
		if (time >= length)
		{
			time -= length;
			loopCount++;
		}
	}

	void reset() { time = 0; }
	int currentFrame() const
	{
		return static_cast<int>((time / length) * frameCount);
	}

	void setTime(float time) { this->time = time; }
	float currentTime() const { return time; }
	int getFrameCount() const { return frameCount; }
	int getLoopCount() const { return loopCount; }
};