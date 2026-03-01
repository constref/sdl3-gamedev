#pragma once

#include <resourceid.h>
#include <string>
#include <cstdint>

class Services;

class ResourceLoader
{
	void *textureLoader;
	void *animLoader;

public:
	ResourceLoader(Services &services);
	ResourceId loadTexture(const std::string &texturePath, bool flip = false);
	ResourceId createAnimation(uint32_t numFrames, float length);
};
