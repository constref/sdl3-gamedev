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
	ResourceId idleAnimationIndex;
	ResourceId idleTexture;
	ResourceId runAnimationIndex;
	ResourceId runTexture;
	ResourceId jumpAnimationIndex;
	ResourceId jumpTexture;
	ResourceId slideAnimationIndex;
	ResourceId slideTexture;
	ResourceId slideShootAnimationIndex;
	ResourceId slideShootTexture;
	ResourceId shootAnimationIndex;
	ResourceId shootTexture;
	ResourceId runShootAnimationIndex;
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
	ResourceId getIdleAnimation() const { return idleAnimationIndex; }
	void setIdleAnimation(ResourceId animId) { idleAnimationIndex = animId; }
	ResourceId getIdleTexture() const { return idleTexture; }
	void setIdleTexture(ResourceId tex) { idleTexture = tex; }
	ResourceId  getRunAnimation() const { return runAnimationIndex; }
	void setRunAnimation(ResourceId animId) { runAnimationIndex = animId; }
	ResourceId getRunTexture() const { return runTexture; }
	void setRunTexture(ResourceId tex) { runTexture = tex; }
	ResourceId getJumpAnimation() const { return jumpAnimationIndex; }
	void setJumpAnimation(ResourceId animId) { jumpAnimationIndex = animId; }
	ResourceId getJumpTexture() const { return jumpTexture; }
	void setJumpTexture(ResourceId tex) { jumpTexture = tex; }
	ResourceId getSlideAnimation() const { return slideAnimationIndex; }
	void setSlideAnimation(ResourceId animId) { slideAnimationIndex = animId; }
	ResourceId getSlideTexture() const { return slideTexture; }
	void setSlideTexture(ResourceId tex) { slideTexture = tex; }
	ResourceId getSlideShootAnimation() const { return slideShootAnimationIndex; }
	void setSlideShootAnimation(ResourceId animId) { slideShootAnimationIndex = animId; }
	ResourceId getSlideShootTexture() const { return slideShootTexture; }
	void setSlideShootTexture(ResourceId tex) { slideShootTexture = tex; }
	ResourceId getShootAnimation() const { return shootAnimationIndex; }
	void setShootAnimation(ResourceId animId) { shootAnimationIndex = animId; }
	ResourceId getShootTexture() const { return shootTexture; }
	void setShootTexture(ResourceId tex) { this->shootTexture = tex; }
	ResourceId getRunShootAnimation() const { return runShootAnimationIndex; }
	void setRunShootAnimation(ResourceId animId) { runShootAnimationIndex = animId; }
	ResourceId getRunShootTexture() const { return runShootTexture; }
	void setRunShootTexture(ResourceId tex) { this->runShootTexture = tex; }
};