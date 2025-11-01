#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <timer.h>

struct SDL_Texture;
class UpdateVelocityCommand;
class UpdateDirectionCommand;
class CollisionEvent;
class FallingEvent;
class JumpEvent;
class ShootBeginEvent;
class ShootEndEvent;

enum class PState
{
	idle,
	shooting,
	running,
	runningShooting,
	airborne,
	airborneShooting,
	falling,
	sliding,
	slidingShooting
};

class PlayerControllerComponent : public Component
{
	float direction;
	glm::vec2 velocity;
	bool grounded;
	bool shooting;
	Timer slideTimer;

	PState currentState;
	int idleAnimationIndex;
	SDL_Texture *idleTexture;
	int runAnimationIndex;
	SDL_Texture *runTexture;
	int slideAnimationIndex;
	SDL_Texture *slideTexture;
	int slideShootAnimationIndex;
	SDL_Texture *slideShootTexture;
	int shootAnimationIndex;
	SDL_Texture *shootTexture;
	int runShootAnimationIndex;
	SDL_Texture *runShootTexture;

public:
	PlayerControllerComponent(Node &owner);
	void update(const FrameContext &ctx) override;
	void onStart() override;
	void transitionState(PState newState);
	void onCommand(const UpdateVelocityCommand &msg);
	void onCommand(const UpdateDirectionCommand &msg);

	void onEvent(const CollisionEvent &event);
	void onEvent(const FallingEvent &event);
	void onEvent(const JumpEvent &event);
	void onEvent(const ShootBeginEvent &event);
	void onEvent(const ShootEndEvent &event);

	void setIdleAnimation(int index) { idleAnimationIndex = index; }
	void setIdleTexture(SDL_Texture *tex) { idleTexture = tex; }
	void setRunAnimation(int index) { runAnimationIndex = index; }
	void setRunTexture(SDL_Texture *tex) { runTexture = tex; }
	void setSlideAnimation(int index) { slideAnimationIndex = index; }
	void setSlideTexture(SDL_Texture *tex) { slideTexture = tex; }
	void setSlideShootAnimation(int index) { slideShootAnimationIndex = index; }
	void setSlideShootTexture(SDL_Texture *tex) { slideShootTexture = tex; }
	void setShootAnimation(int index) { shootAnimationIndex = index; }
	void setShootTexture(SDL_Texture *tex) { this->shootTexture = tex; }
	void setRunShootAnimation(int index) { runShootAnimationIndex = index; }
	void setRunShootTexture(SDL_Texture *tex) { this->runShootTexture = tex; }
};