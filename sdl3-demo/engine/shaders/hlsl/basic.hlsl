#pragma shader_model 5.1

cbuffer cbPerObject : register(b0)
{
    float4x4 worldViewProj;
}

struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PixelIn
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 reflect;
};

PixelIn vsMain(VertexIn input)
{
    PixelIn output;
    output.position = mul(float4(input.position, 1), worldViewProj);
    output.color = input.color;
    return output;
}

float4 psMain(PixelIn input) : SV_TARGET
{
    return input.color;
}

