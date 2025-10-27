#pragma once

#include <components/component.h>
#include <glm/glm.hpp>
#include <memory>
#include <ghandle.h>

class UpdateVelocityCommand;

class BasicCameraComponent : public Component
{
	GHandle target;
	glm::vec2 camPosition;
	glm::vec2 viewportSize;
	glm::vec2 velocity;

public:
	BasicCameraComponent(GameObject &owner, GHandle target, float viewportWidth, float viewportHeight);

	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;
	void update(const FrameContext &ctx) override;
	void onCommand(const UpdateVelocityCommand &msg);
};
