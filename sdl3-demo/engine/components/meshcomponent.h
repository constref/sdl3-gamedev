#pragma once

#include <components/component.h>
#include <uuid.h>

using GPUMeshHandle = size_t;

class MeshComponent : public Component
{
    uuids::uuid m_meshId;

public:
    MeshComponent(Node &owner, uuids::uuid meshId);
    auto meshId() const;
    void setMeshId(uuids::uuid meshId);
};
