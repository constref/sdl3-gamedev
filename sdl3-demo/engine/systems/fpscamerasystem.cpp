#include "fpscamerasystem.h"

FPSCameraSystem::FPSCameraSystem(Services &services) : System(services)
{
}
void FPSCameraSystem::update(Node& node)
{
    auto [ic, cc] = getRequiredComponents(node);
}
