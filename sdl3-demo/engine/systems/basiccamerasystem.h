#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/basiccameracomponent.h>

class BasicCameraSystem : public System<FrameStage::Render, InputComponent, BasicCameraComponent>
{
	glm::vec2 viewportPosition;
	glm::vec2 viewportSize;

public:
	BasicCameraSystem(glm::vec2 viewportSize);
	void update(Node &node);
};