#pragma once

#include <DirectXMath.h>
#include <components/component.h>

class LightingComponent : public Component
{
    DirectX::XMFLOAT4 m_direction;
    DirectX::XMFLOAT4 m_color;
public:
    explicit LightingComponent(Node &owner);

    DirectX::XMFLOAT4 direction() const { return m_direction;}
    void setDirection(DirectX::XMFLOAT4 direction) { m_direction = direction;}
    DirectX::XMFLOAT4 color() const { return m_color;}
    void setColor(DirectX::XMFLOAT4 color) { m_color = color;}
};
