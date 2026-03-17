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
    
    float3x3 normMat = (float3x3)world;
    output.normal = mul(input.normal, normMat);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PSMain(PixelIn input) : SV_TARGET
{
    float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
    
    // ambient light
    float4 ambientColor = float4(0.05, 0.05, 0.05, 1.0);
    finalColor += input.color * ambientColor;
    
    // directional light
    {
        float intensity = 0.1;
        float3 lightDir = normalize(float3(-1, 0, 0));
        float lightAmt = max(dot(lightDir, normalize(input.normal)), 0);
        finalColor += input.color * float4(0.9, 0.8, 0.8, 0) * lightAmt * intensity;
    }
    
    const int numDirLights = 2;
    float3 dirLights[2] = {float3(3, 0.7, -3), float3(1, 0.7, -10)};
    for (int i = 0; i < numDirLights; ++i)
    {
        // point light
        float3 lightPos = dirLights[i];
        const float falloff = 4;
        float3 L = lightPos - input.positionW;
        float distance = length(L);
    
        if (distance < falloff)
        {
            L = L / distance; // normalize
            float lightAmt = max(dot(L, normalize(input.normal)), 0);
            finalColor += input.color * (float4(1.0, 0.2, 0.1, 0.0) * lightAmt) * (1 - smoothstep(falloff - 3, falloff, distance));
        }
    }

    return finalColor;
}

