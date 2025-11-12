#pragma once

#include <glm/glm.hpp>
#include <components/component.h>

struct FrameContext;
class AddImpulseCommand;

class PhysicsComponent : public Component
{
	glm::vec2 velocity;
	glm::vec2 acceleration;
	glm::vec2 netForce;
	glm::vec2 maxSpeed;
	glm::vec2 direction;
	glm::vec2 delta;
	bool grounded;
	bool dynamic;
	float gravityFactor;
	float damping;

public:
	PhysicsComponent(Node &owner);

	glm::vec2 getDirection() const { return direction; }
	void setDirection(const glm::vec2 direction) { this->direction = direction; }
	glm::vec2 getVelocity() const { return velocity; }
	void setVelocity(const glm::vec2 &vel);
	glm::vec2 getAcceleration() const { return acceleration; }
	void setAcceleration(const glm::vec2 &acc) { acceleration = acc; }
	glm::vec2 getMaxSpeed() const { return maxSpeed; }
	void setMaxSpeed(const glm::vec2 &maxSpeed) { this->maxSpeed = maxSpeed; }
	bool isGrounded() const { return grounded; }
	void setGrounded(bool grounded) { this->grounded = grounded; }
	bool isDynamic() const { return dynamic; }
	void setDynamic(bool dynamic) { this->dynamic = dynamic; }
	float getGravityFactor() const { return gravityFactor; }
	void setGravityFactor(float gravityFactor) { this->gravityFactor = gravityFactor; }
	float getDamping() const { return damping; }
	void setDamping(float damping) { this->damping = damping; }
	glm::vec2 getDelta() const { return delta; }
	void setDelta(glm::vec2 delta) { this->delta = delta; }

	void addImpulse(const glm::vec2 &impulse);
};
