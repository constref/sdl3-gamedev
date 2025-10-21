#pragma once

#include <components/component.h>
#include <timer.h>
#include <glm/glm.hpp>

class VelocityMessage;
class DirectionMessage;
class ShootStartMessage;
class ShootEndMessage;

class WeaponComponent : public Component
{
	bool shooting;
	Timer timer;
	float playerDirection;
	glm::vec2 playerVelocity;

public:
	WeaponComponent(GameObject &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(MessageDispatch &msgDispatch) override;

	void onMessage(const VelocityMessage &msg);
	void onMessage(const DirectionMessage &msg);
	void onMessage(const ShootStartMessage &msg);
	void onMessage(const ShootEndMessage &msg);
};