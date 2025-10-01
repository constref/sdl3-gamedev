#pragma once

#include <glm/glm.hpp>
#include "../component.h"
#include "../observer.h"

struct FrameContext;

class PhysicsComponent : public Component
{
	glm::vec2 velocity, acceleration;
	float direction;
	float maxSpeedX;
	bool grounded;

	Subject<glm::vec2> velocitySubject;

public:
	PhysicsComponent(GameObject &owner);
	void update(const FrameContext &ctx);
	void onAttached(SubjectRegistry &registry) override;
	void registerObservers(SubjectRegistry &registry) override;
	void onCommand(const Command &command) override;

	glm::vec2 getVelocity() const { return velocity; }
	void setVelocity(const glm::vec2 &vel)
	{
		velocity = vel;
		velocitySubject.notify(velocity);
	}
	glm::vec2 getAcceleration() const { return acceleration; }
	void setAcceleration(const glm::vec2 &acc)
	{
		acceleration = acc;
	}
	void setMaxSpeed(float maxSpeed) { maxSpeedX = maxSpeed; }
	bool isGrounded() const { return grounded; }
	void setGrounded(bool grounded) { this->grounded = grounded; }
};
