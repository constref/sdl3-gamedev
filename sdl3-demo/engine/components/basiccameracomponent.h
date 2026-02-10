#pragma once

#include <components/component.h>
#include <glm/glm.hpp>
#include <memory>
#include <nodehandle.h>

class BasicCameraComponent : public Component
{
	glm::vec2 viewportPosition;
	glm::vec2 viewportSize;
	int tileWidth = 0;
	int tileHeight = 0;
	int mapWidth = 0;
	int mapHeight = 0;
	glm::vec2 position;

public:
	BasicCameraComponent(Node &owner, glm::vec2 viewportSize, int tileWidth, int tileHeight, int mapWidth, int mapHeigh);

	int getTileWidth() const { return tileWidth; }
	int getTileHeight() const { return tileHeight; }
	glm::vec2 getViewportSize() const { return viewportSize; }

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 &position) { this->position = position; }
};
