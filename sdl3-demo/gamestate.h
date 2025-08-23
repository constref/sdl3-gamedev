#pragma once

#include <vector>
#include <array>
#include "gameobject.h"

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const float TILE_SIZE = 32;

struct GameState
{
	std::array<std::vector<GameObject>, 2> layers;
	std::vector<GameObject> backgroundTiles;
	std::vector<GameObject> foregroundTiles;
	std::vector<GameObject> bullets;
	int playerIndex;
	SDL_FRect mapViewport;
	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	GameState(int logicalWidth, int logicalHeight)
	{
		playerIndex = -1;
		mapViewport = SDL_FRect{
			.x = 0, .y = 0,
			.w = static_cast<float>(logicalWidth),
			.h = static_cast<float>(logicalHeight)
		};
		bg2Scroll = bg3Scroll = bg4Scroll = 0;
		debugMode = false;
	}

	GameObject &player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; }
};

