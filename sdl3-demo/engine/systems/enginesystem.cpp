#include "enginesystem.h"

#include <engine.h>
#include <messaging/events.h>

EngineSystem::EngineSystem(Services &services, Engine &engine) : System(services), m_engine(engine)
{
    services.eventQueue().dispatcher.registerHandler<ShutdownEvent>(this);
}

void EngineSystem::onEvent(NodeHandle target, const ShutdownEvent& event) const
{
    m_engine.stop();
}

void EngineSystem::update(Node& node)
{
}
