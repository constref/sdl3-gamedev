#pragma once

#include <components/component.h>

using GPUMeshHandle = size_t;

class MeshComponent : public Component
{
	GPUMeshHandle handle;

public:
	MeshComponent(Node &owner, GPUMeshHandle meshHandle);
	GPUMeshHandle getHandle() const;
	void setHandle(GPUMeshHandle handle);
};
