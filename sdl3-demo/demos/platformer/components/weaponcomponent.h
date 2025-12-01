#pragma once

#include <components/component.h>
#include <timer.h>
#include <glm/glm.hpp>

class WeaponComponent : public Component
{
	bool shooting;
	bool canFire;
	Timer cooldownTimer;
	float playerDirection;
	glm::vec2 playerVelocity;

public:
	WeaponComponent(Node &owner);

	bool isShooting() const { return shooting; }
	void setIsShooting(bool shooting) { this->shooting = shooting; }

	Timer &getCooldownTimer() { return cooldownTimer; }
};