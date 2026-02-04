#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/basiccameracomponent.h>

class BasicCameraSystem : public System<FrameStage::Gameplay, InputComponent, BasicCameraComponent>
{
	glm::vec2 viewportPosition;
	glm::vec2 viewportSize;
	int tileWidth = 0;
	int tileHeight = 0;
	int mapWidth = 0;
	int mapHeight = 0;

public:
	BasicCameraSystem(Services &services, glm::vec2 viewportSize, int tileWidth, int tileHeight, int mapWidth, int mapHeight);
	void update(Node &node);
};