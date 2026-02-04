#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <timer.h>
#include <resourceid.h>

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
	ResourceId idleTexture;
	int runAnimationIndex;
	ResourceId runTexture;
	int jumpAnimationIndex;
	ResourceId jumpTexture;
	int slideAnimationIndex;
	ResourceId slideTexture;
	int slideShootAnimationIndex;
	ResourceId slideShootTexture;
	int shootAnimationIndex;
	ResourceId shootTexture;
	int runShootAnimationIndex;
	ResourceId runShootTexture;

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
	ResourceId getIdleTexture() const { return idleTexture; }
	void setIdleTexture(ResourceId tex) { idleTexture = tex; }
	int  getRunAnimation() const { return runAnimationIndex; }
	void setRunAnimation(int index) { runAnimationIndex = index; }
	ResourceId getRunTexture() const { return runTexture; }
	void setRunTexture(ResourceId tex) { runTexture = tex; }
	int getJumpAnimation() const { return jumpAnimationIndex; }
	void setJumpAnimation(int index) { jumpAnimationIndex = index; }
	ResourceId getJumpTexture() const { return jumpTexture; }
	void setJumpTexture(ResourceId tex) { jumpTexture = tex; }
	int getSlideAnimation() const { return slideAnimationIndex; }
	void setSlideAnimation(int index) { slideAnimationIndex = index; }
	ResourceId getSlideTexture() const { return slideTexture; }
	void setSlideTexture(ResourceId tex) { slideTexture = tex; }
	int getSlideShootAnimation() const { return slideShootAnimationIndex; }
	void setSlideShootAnimation(int index) { slideShootAnimationIndex = index; }
	ResourceId getSlideShootTexture() const { return slideShootTexture; }
	void setSlideShootTexture(ResourceId tex) { slideShootTexture = tex; }
	int getShootAnimation() const { return shootAnimationIndex; }
	void setShootAnimation(int index) { shootAnimationIndex = index; }
	ResourceId getShootTexture() const { return shootTexture; }
	void setShootTexture(ResourceId tex) { this->shootTexture = tex; }
	int getRunShootAnimation() const { return runShootAnimationIndex; }
	void setRunShootAnimation(int index) { runShootAnimationIndex = index; }
	ResourceId getRunShootTexture() const { return runShootTexture; }
	void setRunShootTexture(ResourceId tex) { this->runShootTexture = tex; }
};