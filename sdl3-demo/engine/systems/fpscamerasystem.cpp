#include "fpscamerasystem.h"

#include "componentsystems.h"
#include "d3d12/d3d12rendersystem.h"
#include "messaging/events.h"

FPSCameraSystem::FPSCameraSystem(Services &services) : System(services)
{
    services.eventQueue().dispatcher.registerHandler<DirectionChangedEvent>(this);
}

void FPSCameraSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event) const
{
}

void FPSCameraSystem::update(Node& node)
{
    auto [ic, cc] = getRequiredComponents(node);
    
    auto *renderSys = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();
    
    glm::vec3 pos = node.getPosition();
    renderSys->setCamPosition(pos.x, pos.y, pos.z);
}
