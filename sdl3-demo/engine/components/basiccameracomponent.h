#pragma once

#include <components/component.h>
#include <glm/glm.hpp>

class VelocityMessage;

class BasicCameraComponent : public Component
{
	glm::vec2 targetPosition;
	glm::vec2 viewportSize;
	glm::vec2 velocity;

public:
	BasicCameraComponent(GameObject &owner, float viewportWidth, float viewportHeight);

	void onAttached(MessageDispatch &msgDispatch) override;
	void update(const FrameContext &ctx) override;
	void onMessage(const VelocityMessage &msg);
};
