#pragma once

enum class FrameStage
{
	Input = 0,
	Physics = 1,
	Gameplay = 2,
	Animation = 3,
	Render = 4,
	Start = 5,
	End = 6,
	StageCount
};
