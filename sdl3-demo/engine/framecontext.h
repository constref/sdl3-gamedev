#pragma once

#include <nodehandle.h>
#include <framestage.h>

struct Resources;
class InputState;

struct FrameContext
{
	float deltaTime;
	double globalTime;
	long frameNumber;
	FrameStage frameStage;

	FrameContext()
	{
		this->deltaTime = 0;
		this->globalTime = 0;
		this->frameNumber = 0;
		this->frameStage = FrameStage::Start;
	}

	static FrameContext &global()
	{
		static FrameContext ctx;
		return ctx;
	}

	static float dt()
	{
		return global().deltaTime;
	}
	static double gt()
	{
		return global().globalTime;
	}
	void setStage(FrameStage stage)
	{
		this->frameStage = stage;
	}
	static FrameStage currentStage()
	{
		return global().frameStage;
	}
};