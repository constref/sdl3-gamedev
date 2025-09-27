#pragma once

#include <glm/glm.hpp>
#include "../component.h"
#include "../observer.h"

struct FrameContext;
class InputComponent;

class PhysicsComponent : public Component
{
	glm::vec2 velocity, acceleration;
	float direction;
	float maxSpeedX;
	bool grounded;

public:
	PhysicsComponent(GameObject &owner, InputComponent *inputComponent = nullptr);
	void update(const FrameContext &ctx);
	void onAttached() override;
	void onCommand(const Command &command) override;

	glm::vec2 getVelocity() const { return velocity; }
	void setVelocity(const glm::vec2 &vel)
	{
		velocity = vel;
		velocityUpdate.notify(velocity);
	}
	glm::vec2 getAcceleration() const { return acceleration; }
	void setAcceleration(const glm::vec2 &acc)
	{
		acceleration = acc;
	}
	void setMaxSpeed(float maxSpeed) { maxSpeedX = maxSpeed; }
	bool isGrounded() const { return grounded; }
	void setGrounded(bool grounded) { this->grounded = grounded; }

	Subject<glm::vec2> velocityUpdate;
};
