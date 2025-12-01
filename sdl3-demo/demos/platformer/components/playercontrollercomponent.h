#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <timer.h>

struct SDL_Texture;
class UpdateVelocityCommand;
class UpdateDirectionCommand;

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
	int jumpAnimationIndex;
	SDL_Texture *jumpTexture;
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
	void onCommand(const UpdateVelocityCommand &msg);
	void onCommand(const UpdateDirectionCommand &msg);

	bool isShooting() const { return shooting; }
	void setIsShooting(bool shooting) { this->shooting = shooting; }
	PState getCurrentState() const{ return currentState; }
	void setCurrentState(PState newState) { currentState = newState; }
	Timer &getSlideTimer() { return slideTimer; }
	int getIdleAnimation() const { return idleAnimationIndex; }
	void setIdleAnimation(int index) { idleAnimationIndex = index; }
	SDL_Texture *getIdleTexture() const { return idleTexture; }
	void setIdleTexture(SDL_Texture *tex) { idleTexture = tex; }
	int  getRunAnimation() const { return runAnimationIndex; }
	void setRunAnimation(int index) { runAnimationIndex = index; }
	SDL_Texture *getRunTexture() const { return runTexture; }
	void setRunTexture(SDL_Texture *tex) { runTexture = tex; }
	int getJumpAnimation() const { return jumpAnimationIndex; }
	void setJumpAnimation(int index) { jumpAnimationIndex = index; }
	SDL_Texture *getJumpTexture() const { return jumpTexture; }
	void setJumpTexture(SDL_Texture *tex) { jumpTexture = tex; }
	int getSlideAnimation() const { return slideAnimationIndex; }
	void setSlideAnimation(int index) { slideAnimationIndex = index; }
	SDL_Texture *getSlideTexture() const { return slideTexture; }
	void setSlideTexture(SDL_Texture *tex) { slideTexture = tex; }
	int getSlideShootAnimation() const { return slideShootAnimationIndex; }
	void setSlideShootAnimation(int index) { slideShootAnimationIndex = index; }
	SDL_Texture *getSlideShootTexture() const { return slideShootTexture; }
	void setSlideShootTexture(SDL_Texture *tex) { slideShootTexture = tex; }
	int getShootAnimation() const { return shootAnimationIndex; }
	void setShootAnimation(int index) { shootAnimationIndex = index; }
	SDL_Texture *getShootTexture() const { return shootTexture; }
	void setShootTexture(SDL_Texture *tex) { this->shootTexture = tex; }
	int getRunShootAnimation() const { return runShootAnimationIndex; }
	void setRunShootAnimation(int index) { runShootAnimationIndex = index; }
	SDL_Texture *getRunShootTexture() const { return runShootTexture; }
	void setRunShootTexture(SDL_Texture *tex) { this->runShootTexture = tex; }
};