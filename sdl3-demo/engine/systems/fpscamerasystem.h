#pragma once

#include <systems/system.h>
#include <components/cameracomponent.h>
#include <components/inputcomponent.h>
#include <messaging/events.h>

class FPSCameraSystem : public System<FrameStage::Gameplay, InputComponent, CameraComponent>
{
public:
    void update(Node& node) override;

    FPSCameraSystem(Services &services);
    
    void onEvent(NodeHandle target, const DirectionChangedEvent& event) const;
};
