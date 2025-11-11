#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <timer.h>

struct SDL_Texture;
class UpdateDirectionCommand;
class FrameChangeCommand;
class UpdateViewportCommand;
class AnimationPlayEvent;

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
	float direction;
	float followViewport;
	glm::vec2 mapViewportPos;
	glm::vec2 mapViewportSize;

public:
	SpriteComponent(Node &owner, SDL_Texture *texture, float width, float height);
	void update() override;

	void onCommand(const UpdateViewportCommand &dp);
	void onEvent(const AnimationPlayEvent &event);

	glm::vec2 getSize() const { return glm::vec2(width, height); }
	glm::vec2 getDirection() const { return glm::vec2(direction, 0); }
	bool isShouldFlash() const { return shouldFlash; }
	void setShouldFlash(bool shouldFlash) { this->shouldFlash = shouldFlash; }
	SDL_Texture *getTexture() const { return texture; }
	Timer &getFlashTimer() { return flashTimer; }
	int getFrameNumber() const { return frameNumber; }
	void setFrameNumber(int frameNumber) { this->frameNumber = frameNumber; }

	void setTexture(SDL_Texture *texture) { this->texture = texture; }
	float getFollowViewport() const { return followViewport; }
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
	void setDirection(float direction) { this->direction = direction; }
};
