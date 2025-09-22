#pragma once

#include <vector>
#include "gameobject.h"

const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const float TILE_SIZE = 32;

struct GameState
{
	std::vector<std::vector<std::shared_ptr<GameObject>>> layers;
	std::vector<GameObject> backgroundTiles;
	std::vector<GameObject> foregroundTiles;
	std::vector<GameObject> bullets;
	int playerIndex;
	int playerLayer;
	SDL_FRect mapViewport;
	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	GameState(int logicalWidth, int logicalHeight)
	{
		playerIndex = -1;
		playerLayer = -1;
		mapViewport = SDL_FRect{
			.x = 0, .y = 0,
			.w = static_cast<float>(logicalWidth),
			.h = static_cast<float>(logicalHeight)
		};
		bg2Scroll = bg3Scroll = bg4Scroll = 0;
		debugMode = false;
	}

	std::shared_ptr<GameObject> &player() { return layers[playerLayer][playerIndex]; }
};

