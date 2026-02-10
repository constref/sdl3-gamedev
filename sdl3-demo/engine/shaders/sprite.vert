#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define M_PI 3.1415926535897932384626433832795
#define M_2PI M_PI * 2

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out flat uint instanceIndex;

struct Vertex
{
    vec3 position;
    vec2 uv;
};

layout(buffer_reference, scalar) readonly buffer VertexPtr
{
    Vertex vertices[];
};

struct InstanceData
{
    VertexPtr vertexAddress;
    float globalTime;
    uint pad1;
    mat4 mvp;
    uint textureIndex;
    uint frameNumber;
    uint frameCount;
    uint width;
    uint height;
    float flipH;
    float layerIndex;
    uint pad2;
};

layout(set = 1, binding = 0, scalar) readonly buffer InstanceBuffer
{
	InstanceData instances[];
};

void main()
{
	InstanceData inst = instances[gl_InstanceIndex];
    vec3 pos = inst.vertexAddress.vertices[gl_VertexIndex].position * vec3(inst.width, inst.height, 1);
    vec2 uv = inst.vertexAddress.vertices[gl_VertexIndex].uv;
    gl_Position = inst.mvp * vec4(pos.x, pos.y, inst.layerIndex, 1);

    outColor = vec3(1, 1, 1);

    float uPortion = 1.0 / inst.frameCount;
    float uStart = uPortion * (inst.frameNumber - 1); // start position in spritesheet
	outUV = vec2((uStart + uPortion * inst.flipH) + (1 - (2 * inst.flipH)) * (uPortion * uv.x), uv.y);

    instanceIndex = gl_InstanceIndex;
}