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
	glm::vec2 scale;
	float rotation;
	int frameNumber;
	float followViewport;
	glm::vec2 viewportPos;
	glm::vec2 viewportSize;
	bool flipH;
	float paralaxFactor;

public:
	SpriteComponent(Node &owner, ResourceId texture, float width, float height);

	glm::vec2 getSize() const { return glm::vec2(width, height); }
	bool isShouldFlash() const { return shouldFlash; }
	void setShouldFlash(bool shouldFlash) { this->shouldFlash = shouldFlash; }
	ResourceId getTexture() const { return texture; }
	void setTexture(ResourceId texture) { this->texture = texture; }
	int getFrameNumber() const { return frameNumber; }
	void setFrameNumber(int frameNumber) { this->frameNumber = frameNumber; }
	bool getFlipMode() const { return flipH; }
	void setFlipMode(bool flipH) { this->flipH = flipH; }
	Timer &getFlashTimer() { return flashTimer; }

	glm::vec2 getViewportPos() const { return viewportPos; }
	void setViewportPos(glm::vec2 viewportPos) { this->viewportPos = viewportPos; }
	float getFollowViewport() const { return followViewport; }
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
	glm::vec2 getScale() const { return scale; }
	void setScale(const glm::vec2 &scale) { this->scale = scale; }
	float getRotation() const { return rotation; }
	void setRotation(float rotation) { this->rotation = rotation; }
	float getParalaxFactor() const { return paralaxFactor; }
	void setParalaxFactor(float factor) { paralaxFactor = factor; }
};
