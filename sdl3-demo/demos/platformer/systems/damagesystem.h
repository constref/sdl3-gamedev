#pragma once

#include <systems/system.h>
#include <components/spritecomponent.h>
#include "../components/healthcomponent.h"

class DamageEvent;

class DamageSystem : public System<FrameStage::Gameplay, HealthComponent, SpriteComponent>
{
public:
	DamageSystem(Services &services);
	void update(Node &node);

	void onEvent(NodeHandle target, const DamageEvent &event);
};