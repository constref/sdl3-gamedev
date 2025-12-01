#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/basiccameracomponent.h>

class BasicCameraSystem : public System<FrameStage::Gameplay, InputComponent, BasicCameraComponent>
{
	glm::vec2 viewportPosition;
	glm::vec2 viewportSize;

public:
	BasicCameraSystem(Services &services, glm::vec2 viewportSize);
	void update(Node &node);
};