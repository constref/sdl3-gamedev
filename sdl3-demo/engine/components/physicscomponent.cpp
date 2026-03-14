#include "physicscomponent.h"

#include <node.h>

PhysicsComponent::PhysicsComponent(Node &owner) : Component(owner)
{
	direction = { 0, 0, 0 };
	maxSpeed = { 0, 0, 0 };
	velocity = { 0, 0, 0 };
	acceleration = { 0, 0, 0 };
	grounded = false;
	netForce = { 0, 0, 0 };
	dynamic = false;
	gravityFactor = 1.0f;
	damping = 10.0f;
	delta = { 0, 0, 0 };
	m_localX = glm::vec3(1, 0, 0);
	m_localY = glm::vec3(0, 1, 0);
	m_localZ = glm::vec3(0, 0, 1);
}

void PhysicsComponent::setVelocity(const glm::vec3 &vel)
{
	this->velocity = vel;
}

void PhysicsComponent::addImpulse(const glm::vec3 &impulse)
{
	velocity += impulse;
}
