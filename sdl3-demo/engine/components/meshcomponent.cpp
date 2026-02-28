#include "meshcomponent.h"

MeshComponent::MeshComponent(Node &owner, GPUMeshHandle meshHandle) : Component(owner, FrameStage::Render)
{
	setHandle(meshHandle);
}

GPUMeshHandle MeshComponent::getHandle() const
{
	return handle;
}

void MeshComponent::setHandle(GPUMeshHandle handle)
{
	this->handle = handle;
}
