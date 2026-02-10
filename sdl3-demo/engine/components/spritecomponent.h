#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <timer.h>
#include <resourceid.h>

class SpriteComponent : public Component
{
protected:
	Timer flashTimer;
	ResourceId texture;
	bool shouldFlash;
	float width;
	float height;
	float spriteSheetWidth;
	float spriteSheetHeight;
	glm::vec3 scale;
	float rotation;
	int frameNumber;
	int frameCount;
	float followViewport;
	glm::vec2 viewportPos;
	glm::vec2 viewportSize;
	bool flipH;
	float paralaxFactor;
	int layerIndex;

public:
	SpriteComponent(Node &owner, ResourceId texture, float width, float height);

	glm::vec2 getSize() const { return glm::vec2(width, height); }
	bool isShouldFlash() const { return shouldFlash; }
	void setShouldFlash(bool shouldFlash) { this->shouldFlash = shouldFlash; }
	ResourceId getTexture() const { return texture; }
	void setTexture(ResourceId texture) { this->texture = texture; }
	int getFrameNumber() const { return frameNumber; }
	void setFrameNumber(int frameNumber) { this->frameNumber = frameNumber; }
	int getFrameCount() const { return frameCount; }
	void setFrameCount(int frameCount) { this->frameCount = frameCount; }
	bool getFlipH() const { return flipH; }
	void setFlipH(bool flipH) { this->flipH = flipH; }
	Timer &getFlashTimer() { return flashTimer; }

	glm::vec2 getViewportPos() const { return viewportPos; }
	void setViewportPos(glm::vec2 viewportPos) { this->viewportPos = viewportPos; }
	float getFollowViewport() const { return followViewport; }
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
	glm::vec3 getScale() const { return scale; }
	void setScale(const glm::vec3 &scale) { this->scale = scale; }
	float getRotation() const { return rotation; }
	void setRotation(float rotation) { this->rotation = rotation; }
	float getParalaxFactor() const { return paralaxFactor; }
	void setParalaxFactor(float factor) { paralaxFactor = factor; }
	int getLayerIndex() const { return layerIndex; }
	void setLayerIndex(int index) { this->layerIndex = index; }
};
