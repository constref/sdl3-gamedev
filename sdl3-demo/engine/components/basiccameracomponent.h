#pragma once

#include <components/component.h>
#include <glm/glm.hpp>
#include <memory>
#include <nodehandle.h>

class UpdateVelocityCommand;

class BasicCameraComponent : public Component
{
	NodeHandle target;
	glm::vec2 camPosition;
	glm::vec2 viewportSize;
	glm::vec2 velocity;

public:
	BasicCameraComponent(Node &owner, NodeHandle target, float viewportWidth, float viewportHeight);

	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;
	void update(const FrameContext &ctx) override;
	void onCommand(const UpdateVelocityCommand &msg);
};
