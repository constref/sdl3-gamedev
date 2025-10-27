#pragma once

#include <components/component.h>
#include <timer.h>
#include <glm/glm.hpp>

class VelocityDPump;
class DirectionDPump;
class ShootBeginEvent;
class ShootEndEvent;

class WeaponComponent : public Component
{
	bool shooting;
	Timer timer;
	float playerDirection;
	glm::vec2 playerVelocity;

public:
	WeaponComponent(GameObject &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onData(const VelocityDPump &dp);
	void onData(const DirectionDPump &dp);
	void onEvent(const ShootBeginEvent &event);
	void onEvent(const ShootEndEvent &event);
};