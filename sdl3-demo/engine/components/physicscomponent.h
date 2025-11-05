#pragma once

#include <glm/glm.hpp>
#include <components/component.h>

struct FrameContext;
class ScaleVelocityAxisCommand;
class AddImpulseCommand;
class UpdateDirectionCommand;

class PhysicsComponent : public Component
{
	glm::vec2 velocity;
	glm::vec2 acceleration;
	glm::vec2 netForce;
	glm::vec2 maxSpeed;
	float direction;
	bool grounded;
	bool dynamic;
	float gravityFactor;
	float damping;

public:
	PhysicsComponent(Node &owner);
	void update();

	glm::vec2 getVelocity() const { return velocity; }
	void setVelocity(const glm::vec2 &vel);
	glm::vec2 getAcceleration() const { return acceleration; }
	void setAcceleration(const glm::vec2 &acc)
	{
		acceleration = acc;
	}
	void setMaxSpeed(const glm::vec2 &maxSpeed) { this->maxSpeed = maxSpeed; }
	bool isGrounded() const { return grounded; }
	void setGrounded(bool grounded) { this->grounded = grounded; }
	bool isDynamic() const { return dynamic; }
	void setDynamic(bool dynamic) { this->dynamic = dynamic; }
	float getGravityFactor() const { return gravityFactor; }
	void setGravityFactor(float gravityFactor) { this->gravityFactor = gravityFactor; }
	void setDamping(float damping) { this->damping = damping; }

	void onCommand(const ScaleVelocityAxisCommand &msg);
	void onCommand(const AddImpulseCommand &msg);
	void onCommand(const UpdateDirectionCommand &msg);
};
