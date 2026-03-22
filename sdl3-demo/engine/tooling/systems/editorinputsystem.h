#pragma once
#include <systems/system.h>
#include <components/inputcomponent.h>
#include <messaging/events.h>

class EditorInputSystem : public System<FrameStage::Input, InputComponent>
{
public:
    EditorInputSystem(Services& services);
    void onEvent(NodeHandle target, const KeyUpEvent& event) const;
    void update(Node& node) override;
};
