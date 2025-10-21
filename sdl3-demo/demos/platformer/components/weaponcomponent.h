#pragma once

#include <components/component.h>

class ShootStartMessage;
class ShootEndMessage;

class WeaponComponent : public Component
{
	bool shooting;

public:
	WeaponComponent(GameObject &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(MessageDispatch &msgDispatch) override;

	void onMessage(const ShootStartMessage &msg);
	void onMessage(const ShootEndMessage &msg);
};