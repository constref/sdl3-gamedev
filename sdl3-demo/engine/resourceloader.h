#pragma once

#include <resourceid.h>
#include <string>
#include <engineapi.h>

class Services;
class Loader;

class ENGINE_API ResourceLoader
{
	Loader *textureLoader;
	Loader *animLoader;

public:
	ResourceLoader(Services &services);
	ResourceId loadTexture(const std::string &texturePath, bool flip = false);
	ResourceId createAnimation(uint32_t numFrames, float length);
};
