#include "meshcomponent.h"

MeshComponent::MeshComponent(Node &owner, GPUMeshHandle meshHandle) : Component(owner)
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
