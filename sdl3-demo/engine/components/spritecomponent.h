#pragma once

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <components/component.h>
#include <timer.h>

class SpriteComponent : public Component
{
protected:
	Timer flashTimer;
	SDL_Texture *texture;
	bool shouldFlash;
	float width;
	float height;
	glm::vec2 scale;
	int frameNumber;
	float followViewport;
	glm::vec2 viewportPos;
	glm::vec2 viewportSize;
	SDL_FlipMode flipMode;

public:
	SpriteComponent(Node &owner, SDL_Texture *texture, float width, float height);

	glm::vec2 getSize() const { return glm::vec2(width, height); }
	bool isShouldFlash() const { return shouldFlash; }
	void setShouldFlash(bool shouldFlash) { this->shouldFlash = shouldFlash; }
	SDL_Texture *getTexture() const { return texture; }
	Timer &getFlashTimer() { return flashTimer; }
	int getFrameNumber() const { return frameNumber; }
	void setFrameNumber(int frameNumber) { this->frameNumber = frameNumber; }
	void setTexture(SDL_Texture *texture) { this->texture = texture; }
	SDL_FlipMode getFlipMode() const { return flipMode; }
	void setFlipMode(SDL_FlipMode mode) { flipMode = mode; }

	glm::vec2 getViewportPos() const { return viewportPos; }
	void setViewportPos(glm::vec2 viewportPos) { this->viewportPos = viewportPos; }
	float getFollowViewport() const { return followViewport; }
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }

};
