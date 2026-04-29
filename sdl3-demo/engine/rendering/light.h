#pragma once

#include <DirectXMath.h>

struct Light
{
    DirectX::XMFLOAT4 position;
    DirectX::XMFLOAT4 direction;
    DirectX::XMFLOAT4 color;
    float falloff;
};

