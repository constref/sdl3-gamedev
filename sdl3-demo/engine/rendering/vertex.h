#pragma once

#include <DirectXMath.h>

struct Vertex
{
    DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3 normal = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1);
    DirectX::XMFLOAT2 uv = DirectX::XMFLOAT2(0, 0);
};
