#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <timer.h>

struct SDL_Texture;
class UpdateDirectionCommand;
class FrameChangeCommand;
class UpdateViewportCommand;
class AnimationPlayEvent;

class RenderComponent : public Component
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
	RenderComponent(Node &owner, SDL_Texture *texture, float width, float height);
	void update() override;

	void onCommand(const UpdateDirectionCommand &dp);
	void onCommand(const FrameChangeCommand &dp);
	void onCommand(const UpdateViewportCommand &dp);
	void onEvent(const AnimationPlayEvent &event);

	void setTexture(SDL_Texture *texture) { this->texture = texture; }
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
	void setDirection(float direction) { this->direction = direction; }
};
