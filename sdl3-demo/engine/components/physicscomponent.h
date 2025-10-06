#pragma once

#include <glm/glm.hpp>
#include <components/component.h>
#include <messaging/observer.h>

struct FrameContext;
class IntegrateVelocityMessage;
class ScaleVelocityAxisMessage;
class AddImpulseMessage;

class PhysicsComponent : public Component
{
	glm::vec2 velocity;
	glm::vec2 acceleration;
	glm::vec2 netForce;
	float direction;
	float maxSpeedX;
	bool grounded;

	Subject<glm::vec2> velocitySubject;

public:
	PhysicsComponent(GameObject &owner);
	void update(const FrameContext &ctx);
	void onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch) override;
	void registerObservers(SubjectRegistry &registry) override;

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

	void onMessage(const IntegrateVelocityMessage &msg);
	void onMessage(const ScaleVelocityAxisMessage &msg);
	void onMessage(const AddImpulseMessage &msg);
};
