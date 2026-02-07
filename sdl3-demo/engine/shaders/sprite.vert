#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define M_PI 3.1415926535897932384626433832795
#define M_2PI M_PI * 2

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex
{
    vec3 position;
    vec2 uv;
};

layout(buffer_reference, scalar) readonly buffer VertexPtr
{
    Vertex vertices[];
};

layout(push_constant, scalar) uniform DrawConstants
{
    uint64_t vertexAddress;
    float globalTime;
    float padding;
    mat4 mvp;
    uint textureIndex;
    uint frameNumber;
    uint frameCount;
    uint width;
    uint height;
    float flipH;
    float layerIndex;
} dc;

void main()
{
    VertexPtr vBuffer = VertexPtr(dc.vertexAddress);
    vec3 pos = vBuffer.vertices[gl_VertexIndex].position * vec3(dc.width, dc.height, 1);
    vec2 uv = vBuffer.vertices[gl_VertexIndex].uv;
    gl_Position = dc.mvp * vec4(pos, dc.layerIndex);
    gl_Position = dc.mvp * vec4(pos, 1);

    outColor = vec3(1, 1, 1);

    float uPortion = dc.flipH * (1.0 / dc.frameCount);
    float uStart = uPortion * (dc.frameNumber - 1); // start position in spritesheet

    outUV = vec2(uStart + uPortion * uv.x, uv.y);
}