#include "fpscamerasystem.h"

#include "componentsystems.h"
#include "d3d12/d3d12rendersystem.h"
#include "messaging/events.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

FPSCameraSystem::FPSCameraSystem(Services &services) : System(services)
{
    services.eventQueue().dispatcher.registerHandler<DirectionChangedEvent>(this);
	services.eventQueue().dispatcher.registerHandler<MouseMotionEvent>(this);
}

void FPSCameraSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event) const
{
}

void FPSCameraSystem::onEvent(NodeHandle target, const MouseMotionEvent& event)
{
	using namespace DirectX;
	Node &node = services.world().getNode(target);
	auto [ic, cc, pc] = getRequiredComponents(node);
	rotY += ic->mouseDelta().x * FrameContext::dt();
	// rotate local z around y axis
	glm::vec3 localZ = pc->localZ();
	glm::vec3 localY = pc->localY();
	glm::vec3 newLocalZ = glm::rotateY(localZ, ic->mouseDelta().x * FrameContext::dt());
	pc->setLocalZ(glm::normalize(newLocalZ));
	
	// calculate new local x axis
	glm::vec3 newLocalX = glm::cross(newLocalZ, localY);
	//pc->setLocalX(newLocalX);
    
	Logger::info(this, std::format("{}", rotY));
}

void FPSCameraSystem::update(Node& node)
{
    auto [ic, cc, pc] = getRequiredComponents(node);
    auto *renderSys = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();
    glm::vec3 pos = node.getPosition();
	renderSys->setCamDirection(pc->localZ().x, pc->localZ().y, pc->localZ().z);
    renderSys->setCamPosition(pos.x, pos.y, pos.z);
}
