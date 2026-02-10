#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>
#include <components/basiccameracomponent.h>

class BasicCameraSystem : public System<FrameStage::Gameplay, InputComponent, BasicCameraComponent>
{

public:
	BasicCameraSystem(Services &services);
	void update(Node &node);
};