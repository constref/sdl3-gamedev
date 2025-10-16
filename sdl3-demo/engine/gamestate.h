#pragma once

#include <vector>
#include "gameobject.h"

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const float TILE_SIZE = 32;

struct GameState
{
	std::vector<std::vector<std::shared_ptr<GameObject>>> layers;

	GameState(int logicalWidth, int logicalHeight)
	{
	}
};

