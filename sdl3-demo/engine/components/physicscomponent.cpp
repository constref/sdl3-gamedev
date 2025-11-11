#include "physicscomponent.h"

#include <node.h>
#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/events.h>

PhysicsComponent::PhysicsComponent(Node &owner) : Component(owner, FrameStage::Physics)
{
	direction = { 0, 0 };
	maxSpeed = { 0, 0 };
	velocity = { 0, 0 };
	acceleration = { 0, 0 };
	grounded = false;
	netForce = { 0, 0 };
	dynamic = false;
	gravityFactor = 1.0f;
	damping = 10.0f;
	delta = { 0, 0 };
}

void PhysicsComponent::setVelocity(const glm::vec2 &vel)
{
	this->velocity = vel;
	//owner.sendCommand(UpdateVelocityCommand{ vel });
}
//
//void PhysicsComponent::onCommand(const ScaleVelocityAxisCommand &msg)
//{
//	glm::vec2 vel = getVelocity();
//	vel[static_cast<int>(msg.getAxis())] *= msg.getFactor();
//	setVelocity(vel);
//}
//
//void PhysicsComponent::onCommand(const AddImpulseCommand &msg)
//{
//	glm::vec2 vel = getVelocity();
//	vel += msg.getImpulse();
//	setVelocity(vel);
//}
