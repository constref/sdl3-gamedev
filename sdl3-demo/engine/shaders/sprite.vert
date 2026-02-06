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
} drawConsts;

void main()
{
    VertexPtr vBuffer = VertexPtr(drawConsts.vertexAddress);
    vec3 pos = vBuffer.vertices[gl_VertexIndex].position * vec3(drawConsts.width, drawConsts.height, 1);
    vec2 uv = vBuffer.vertices[gl_VertexIndex].uv;
    gl_Position = drawConsts.mvp * vec4(pos, 1.0);

	const vec3 colors[3] = vec3[]
	(
		vec3(1.0, 0.0, 0.0), // Red
		vec3(0.0, 1.0, 0.0), // Green
		vec3(0.0, 0.0, 1.0)  // Blue
	);
    outColor = colors[gl_InstanceIndex % 3];
    outUV = vec2(uv.x, uv.y);
}