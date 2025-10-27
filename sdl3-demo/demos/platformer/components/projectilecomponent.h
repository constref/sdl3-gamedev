#pragma once

#include <components/component.h>

class ProjectileComponent : public Component
{
public:
	ProjectileComponent(GameObject &owner);

	void update(const FrameContext &ctx) override;
	void onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;
};