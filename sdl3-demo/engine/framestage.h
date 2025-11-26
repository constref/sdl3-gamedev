#pragma once

enum class FrameStage
{
	Start = 0,
	Input = 1,
	Physics = 2,
	Gameplay = 3,
	Animation = 4,
	Render = 5,
	End = 6,
	StageCount
};
