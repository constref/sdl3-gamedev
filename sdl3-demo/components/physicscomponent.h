#pragma once

#include <glm/glm.hpp>
#include "component.h"
#include "observer.h"

struct FrameContext;
class InputComponent;

class PhysicsComponent : public Component
{
	glm::vec2 velocity, acceleration;
	float direction;
	float maxSpeedX;
	bool dynamic;
	bool grounded;

public:
	PhysicsComponent(InputComponent *inputComponent);
	void update(GameObject &owner, const FrameContext &ctx);

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
	bool isDynamic() const { return dynamic; }
	void setDynamic(bool dyn) { dynamic = dyn; }

	Subject<glm::vec2> velocityUpdate;
};
