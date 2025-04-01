#pragma once

#include <glm/glm.hpp>

struct SDL_Texture;

struct GameObject
{
	SDL_Texture *texture;
	glm::vec2 position, velocity, acceleration;

	GameObject()
		: texture(nullptr), position(0, 0), velocity(0, 0), acceleration(0, 0)
	{
	}
};