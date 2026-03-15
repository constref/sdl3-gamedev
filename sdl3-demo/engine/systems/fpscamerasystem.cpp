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
	
	// rotate local z around y axis
	XMFLOAT3 localX;
	XMFLOAT3 localY = pc->localY();
	XMFLOAT3 localZ = pc->localZ();
	
	const XMVECTOR oldLocalZ = XMLoadFloat3(&localZ);
	const XMVECTOR rotAxisY = XMLoadFloat3(&localY);
	const XMVECTOR quatRotY = XMQuaternionRotationAxis(rotAxisY, static_cast<float>(event.xRel()) * 0.5f * FrameContext::dt());
	XMVECTOR newLocalZ = XMVector3Rotate(oldLocalZ, quatRotY);
	
	// calculate new local x axis
	const XMVECTOR vecLocalY = XMLoadFloat3(&localY);
	XMVECTOR newLocalX = XMVector3Cross(vecLocalY, newLocalZ);
	XMStoreFloat3(&localX, newLocalX);
	pc->setLocalX(localX);
	
	const XMVECTOR rotAxisX = XMLoadFloat3(&localX);
	const XMVECTOR quatRotX = XMQuaternionRotationAxis(rotAxisX, static_cast<float>(event.yRel()) * 0.5f * FrameContext::dt());
	newLocalZ = XMVector3Rotate(newLocalZ, quatRotX);
	
	// finally set local z
	XMStoreFloat3(&localZ, newLocalZ);
	pc->setLocalZ(localZ);
	
	// update the render system camera details
	auto *renderSys = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();
	renderSys->setCamDirection(pc->localZ().x, pc->localZ().y, pc->localZ().z);
}

void FPSCameraSystem::update(Node& node)
{
	auto *renderSys = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();
	glm::vec3 pos = node.getPosition();
	renderSys->setCamPosition(pos.x, pos.y, pos.z);
}
