#pragma once

#include <components/component.h>
#include <glm/glm.hpp>
#include <memory>
#include <ghandle.h>

class VelocityMessage;

class BasicCameraComponent : public Component
{
	GHandle target;
	glm::vec2 camPosition;
	glm::vec2 viewportSize;
	glm::vec2 velocity;

public:
	BasicCameraComponent(GameObject &owner, GHandle target, float viewportWidth, float viewportHeight);

	void onAttached(MessageDispatch &msgDispatch) override;
	void update(const FrameContext &ctx) override;
	void onMessage(const VelocityMessage &msg);
};
