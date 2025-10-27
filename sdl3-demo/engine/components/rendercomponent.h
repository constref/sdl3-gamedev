#pragma once

#include <glm/glm.hpp>
#include "component.h"
#include "../timer.h"

struct SDL_Texture;
class SetAnimationCommand;
class UpdateDirectionCommand;
class FrameChangeCommand;
class UpdateViewportCommand;

class RenderComponent : public Component
{
	Timer flashTimer;
	SDL_Texture *texture;
	bool shouldFlash;
	float width;
	float height;
	int frameNumber;
	float direction;
	float followViewport;
	static glm::vec2 mapViewportPos;
	static glm::vec2 mapViewportSize;

public:
	RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height);
	void update(const FrameContext &ctx) override;
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;

	void onCommand(const SetAnimationCommand &dp);
	void onCommand(const UpdateDirectionCommand &dp);
	void onCommand(const FrameChangeCommand &dp);
	void onCommand(const UpdateViewportCommand &dp);

	void setTexture(SDL_Texture *texture) { this->texture = texture; }
	void setFollowViewport(bool shouldFollow) { followViewport = shouldFollow ? 1.0f : 0.0f; }
};
