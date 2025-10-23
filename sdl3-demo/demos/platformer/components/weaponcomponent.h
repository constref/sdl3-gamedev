#pragma once

#include <components/component.h>
#include <timer.h>
#include <glm/glm.hpp>

class VelocityDPump;
class DirectionDPump;
class ShootStartDPump;
class ShootEndDPump;

class WeaponComponent : public Component
{
	bool shooting;
	Timer timer;
	float playerDirection;
	glm::vec2 playerVelocity;

public:
	WeaponComponent(GameObject &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(DataDispatcher &dataDispatcher) override;

	void onData(const VelocityDPump &dp);
	void onData(const DirectionDPump &dp);
	void onData(const ShootStartDPump &dp);
	void onData(const ShootEndDPump &dp);
};