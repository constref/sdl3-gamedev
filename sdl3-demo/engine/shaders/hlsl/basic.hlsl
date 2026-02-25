#pragma shader_model 5.1

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PixelIn
{
    float4 positionW : POSITION;
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
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
    output.positionW = mul(float4(input.position, 1), modelMatrix);
    output.position = mul(float4(input.position, 1), mvpMatrix);
    output.normal = normalize(mul((float3x3) invTransposeMatrix, input.normal));
    output.color = float4(1, 1, 1, 1);

    return output;
}

float4 psMain(PixelIn input) : SV_TARGET
{
    return float4(1, 1, 1, 1);

}

