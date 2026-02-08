#pragma once

#include <glm/glm.hpp>
#include <components/component.h>

struct FrameContext;
class AddImpulseCommand;

class PhysicsComponent : public Component
{
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 netForce;
	glm::vec3 maxSpeed;
	glm::vec3 direction;
	glm::vec3 delta;
	bool grounded;
	bool dynamic;
	float gravityFactor;
	float damping;

public:
	PhysicsComponent(Node &owner);

	glm::vec3 getDirection() const { return direction; }
	void setDirection(const glm::vec3 direction) { this->direction = direction; }
	glm::vec3 getVelocity() const { return velocity; }
	void setVelocity(const glm::vec3 &vel);
	glm::vec3 getAcceleration() const { return acceleration; }
	void setAcceleration(const glm::vec3 &acc) { acceleration = acc; }
	glm::vec3 getMaxSpeed() const { return maxSpeed; }
	void setMaxSpeed(const glm::vec3 &maxSpeed) { this->maxSpeed = maxSpeed; }
	bool isGrounded() const { return grounded; }
	void setGrounded(bool grounded) { this->grounded = grounded; }
	float getGravityFactor() const { return gravityFactor; }
	void setGravityFactor(float gravityFactor) { this->gravityFactor = gravityFactor; }
	float getDamping() const { return damping; }
	void setDamping(float damping) { this->damping = damping; }
	glm::vec3 getDelta() const { return delta; }
	void setDelta(glm::vec3 delta) { this->delta = delta; }

	void addImpulse(const glm::vec3 &impulse);
};
