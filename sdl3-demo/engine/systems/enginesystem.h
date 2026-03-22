#pragma once

#include <systems/system.h>
#include <components/enginecomponent.h>

class Engine;
class ShutdownEvent;

class EngineSystem : public System<FrameStage::End, EngineComponent>
{
    Engine &m_engine;
public:
    EngineSystem(Services &services, Engine &engine);
    
    void onEvent(NodeHandle target, const ShutdownEvent &event) const;
    void update(Node& node) override;
};
