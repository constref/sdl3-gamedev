#include "lightingsystem.h"

LightingSystem::LightingSystem(Services &services) : System(services) {}

void LightingSystem::update(Node &node)
{
    auto [lc] = getRequiredComponents(node);

    glm::vec3 pos = node.getPosition();

    m_lights[m_lightCount++] =
        Light{.position = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f), .color = lc->color(), .falloff = 5};
}

void LightingSystem::beginFrame() { m_lightCount = 0; }

size_t LightingSystem::lightsByteSize() const { return numLights() * sizeof(Light); }
