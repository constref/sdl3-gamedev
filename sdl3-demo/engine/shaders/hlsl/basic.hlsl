cbuffer cbPerObject : register(b0)
{
    float4x4 worldViewProj;
}

struct VertexIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PixelIn
{
    float4 position : SV_Position;
    float4 normal : NORMAL;
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

PixelIn VSMain(VertexIn input)
{
    PixelIn output;
    output.position = mul(float4(input.position, 1), worldViewProj);
    output.normal = float4(normalize(input.normal), 1);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PSMain(PixelIn input) : SV_TARGET
{
    return input.normal;
}

