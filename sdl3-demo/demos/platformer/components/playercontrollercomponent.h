#pragma once

#include <glm/glm.hpp>
#include <components/component.h>

struct SDL_Texture;
class UpdateVelocityCommand;
class UpdateDirectionCommand;
class CollisionEvent;
class FallingEvent;
class JumpEvent;

enum class PState
{
	idle,
	running,
	airborne,
	falling,
	sliding
};

class PlayerControllerComponent : public Component
{
	float direction;
	glm::vec2 velocity;
	bool grounded;

	PState currentState;
	int idleAnimationIndex;
	SDL_Texture *idleTexture;
	int runAnimationIndex;
	SDL_Texture *runTexture;
	int slideAnimationIndex;
	SDL_Texture *slideTexture;

public:
	PlayerControllerComponent(Node &owner);
	void update(const FrameContext &ctx) override;
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;
	void onStart() override;
	void transitionState(PState newState);
	void onCommand(const UpdateVelocityCommand &msg);
	void onCommand(const UpdateDirectionCommand &msg);

	void onEvent(const CollisionEvent &event);
	void onEvent(const FallingEvent &event);
	void onEvent(const JumpEvent &event);

	void setIdleAnimation(int index) { idleAnimationIndex = index; }
	void setIdleTexture(SDL_Texture *tex) { idleTexture = tex; }
	void setRunAnimation(int index) { runAnimationIndex = index; }
	void setRunTexture(SDL_Texture *tex) { runTexture = tex; }
	void setSlideAnimation(int index) { slideAnimationIndex = index; }
	void setSlideTexture(SDL_Texture *tex) { slideTexture = tex; }
};