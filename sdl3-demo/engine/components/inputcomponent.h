#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <nodehandle.h>

class InputComponent : public Component
{
	NodeHandle ownerHandle;
	glm::vec3 direction;

public:
	InputComponent(Node &owner, NodeHandle ownerHandle);

	glm::vec3 getDirection() const { return direction; }
	void setDirection(const glm::vec3 &direction) { this->direction = direction; }
};