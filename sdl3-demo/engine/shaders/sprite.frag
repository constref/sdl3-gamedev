#version 460

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

// output to the first color attachment (swapchain)
layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint instanceIndex;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D textures[];

struct InstanceData
{
    uint64_t vertexAddress;
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

layout(set = 1, binding = 0) readonly buffer InstanceBuffer
{
	InstanceData instances[];
};

void main()
{
	//float brightness = 1.0 - gl_FragCoord.z;
    vec4 s = texture(textures[instances[instanceIndex].textureIndex], inUV);
	fragColor = s;
}