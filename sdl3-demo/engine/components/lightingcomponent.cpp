#include "lightingcomponent.h"

LightingComponent::LightingComponent(Node &owner) : Component(owner)
{
    m_direction = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}
