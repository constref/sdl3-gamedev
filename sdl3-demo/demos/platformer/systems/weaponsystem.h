#pragma once

#include <systems/system.h>

#include "../components/weaponcomponent.h"

class WeaponSystem : public System<FrameStage::Gameplay, WeaponComponent>
{
public:
	void update(Node &node);
};