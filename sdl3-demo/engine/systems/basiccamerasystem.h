#pragma once

#include <systems/system.h>
#include <components/inputcomponent.h>

class BasicCameraSystem : public System<FrameStage::Render, InputComponent>
{

};