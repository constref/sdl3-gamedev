#pragma once

#include <glm/glm.hpp>
#include <DirectXMath.h>
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
	DirectX::XMFLOAT3 m_localX;
	DirectX::XMFLOAT3 m_localY;
	DirectX::XMFLOAT3 m_localZ;
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
	DirectX::XMFLOAT3 localX() const { return m_localX; }
	void setLocalX(const DirectX::XMFLOAT3 &localX) { m_localX = localX; }
	DirectX::XMFLOAT3 localY() const { return m_localY; }
	void setLocalY(const DirectX::XMFLOAT3 &localY) { m_localY = localY; }
	DirectX::XMFLOAT3 localZ() const { return m_localZ; }
	void setLocalZ(const DirectX::XMFLOAT3 &localZ) { m_localZ = localZ; }

	void addImpulse(const glm::vec3 &impulse);
};
