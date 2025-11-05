#pragma once

#include <nodehandle.h>

struct Resources;
class InputState;

struct FrameContext
{
	float deltaTime;
	double globalTime;
	long frameNumber;

	FrameContext()
	{
		this->deltaTime = 0;
		this->globalTime = 0;
		this->frameNumber = 0;
	}

	static FrameContext &global()
	{
		static FrameContext ctx;
		return ctx;
	}
};