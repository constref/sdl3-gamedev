#pragma once

#include <glm/glm.hpp>
#include <components/component.h>

struct FrameContext;
class ScaleVelocityAxisMessage;
class AddImpulseMessage;
class DirectionMessage;

class PhysicsComponent : public Component
{
	glm::vec2 velocity;
	glm::vec2 acceleration;
	glm::vec2 netForce;
	glm::vec2 maxSpeed;
	float direction;
	bool grounded;
	bool dynamic;
	bool hasCollider;

public:
	PhysicsComponent(GameObject &owner);
	void update(const FrameContext &ctx);
	void onAttached(MessageDispatch &msgDispatch) override;

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

	void onMessage(const ScaleVelocityAxisMessage &msg);
	void onMessage(const AddImpulseMessage &msg);
	void onMessage(const DirectionMessage &msg);
};
