#include "resourceloader.h"

#include <componentsystems.h>
#include <systems/vulkanrendersystem.h>
#include <systems/spriteanimationsystem.h>

ResourceLoader::ResourceLoader(Services &services)
{
	textureLoader = services.compSys().getSystemRegistry().getSystem<vks::VulkanRenderSystem>();
	animLoader = services.compSys().getSystemRegistry().getSystem<SpriteAnimationSystem>();
}

ResourceId ResourceLoader::loadTexture(const std::string &texturePath, bool flip)
{
	vks::VulkanRenderSystem *renderSys = static_cast<vks::VulkanRenderSystem *>(textureLoader);
	return renderSys->loadTexture(texturePath, flip);
}

ResourceId ResourceLoader::createAnimation(uint32_t numFrames, float length)
{
	SpriteAnimationSystem *animSys = static_cast<SpriteAnimationSystem *>(animLoader);
	return animSys->createAnimation(numFrames, length);
}
