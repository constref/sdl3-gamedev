#pragma once

#include <components/component.h>
#include <timer.h>
#include <glm/glm.hpp>

class UpdateVelocityCommand;
class UpdateDirectionCommand;
class ShootBeginEvent;
class ShootEndEvent;

class WeaponComponent : public Component
{
	bool shooting;
	Timer timer;
	float playerDirection;
	glm::vec2 playerVelocity;

public:
	WeaponComponent(Node &owner);

	void update() override;
	void onCommand(const UpdateVelocityCommand &dp);
	void onCommand(const UpdateDirectionCommand &dp);
	void onEvent(const ShootBeginEvent &event);
	void onEvent(const ShootEndEvent &event);
};