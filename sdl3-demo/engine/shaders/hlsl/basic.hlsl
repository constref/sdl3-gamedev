struct RenderObject 
{
    uint baseMatrixIndex;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 reflect;
};

StructuredBuffer<float4x4> matrices : register(t0);
StructuredBuffer<RenderObject> renderObjects : register(t1);

uint roIdx : register(b0);

cbuffer cbPerFrame : register(b1)
{
    float4x4 viewProj;
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
    float4 positionW : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

PixelIn VSMain(VertexIn input)
{
    RenderObject ro = renderObjects[roIdx];
    float4x4 world = matrices[ro.baseMatrixIndex];
    float4x4 worldViewProj = matrices[ro.baseMatrixIndex + 1];
    float4x4 invTransWorld = matrices[ro.baseMatrixIndex + 2];

    PixelIn output;
    output.position = mul(float4(input.position, 1), worldViewProj);
    output.positionW = mul(float4(input.position, 1), world);
    output.normal = mul(float4(input.normal, 0), invTransWorld).xyz;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PSMain(PixelIn input) : SV_TARGET
{
    float3 lightPos = float3(2, 0.5f, 1);
    float3 lightTarget = float3(0, 0, 0);
    float3 lightDir = lightTarget - lightPos;

    // ambient light
    float4 ambientColor = float4(0.1, 0.1, 0.1, 1.0);
    
    // point light
    float3 L = normalize(lightPos - input.positionW);
    float lightAmt = max(dot(L, normalize(input.normal)), 0);
    float4 lightColor = float4(0.8, 0.6, 0.5, 1.0);

    return (input.color * ambientColor) + (input.color * (lightColor * lightAmt));
}

