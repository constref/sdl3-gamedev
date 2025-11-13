#pragma once

#include <components/component.h>
#include <glm/glm.hpp>
#include <memory>
#include <nodehandle.h>

class BasicCameraComponent : public Component
{
	glm::vec2 position;

public:
	BasicCameraComponent(Node &owner);

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 &position) { this->position = position; }
};
