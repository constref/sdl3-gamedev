#include "meshcomponent.h"
#include "component.h"

MeshComponent::MeshComponent(Node &owner, uuids::uuid meshId) : Component(owner) { m_meshId = meshId; }
uuids::uuid MeshComponent::meshId() const { return m_meshId; }
void MeshComponent::setMeshId(uuids::uuid meshId) { m_meshId = meshId; }
