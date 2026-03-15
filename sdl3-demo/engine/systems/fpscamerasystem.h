#pragma once

#include <systems/system.h>
#include <components/cameracomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <messaging/events.h>

class FPSCameraSystem : public System<FrameStage::Gameplay, InputComponent, CameraComponent, PhysicsComponent>
{
public:
    FPSCameraSystem(Services& services);
    void update(Node& node) override;
    void onEvent(NodeHandle target, const DirectionChangedEvent& event) const;
    void onEvent(NodeHandle target, const MouseMotionEvent& event);
};
