#pragma once

#include <glm/glm.hpp>
#include "message.h"

struct SDL_Texture;

class SetAnimationMessage : public Message<SetAnimationMessage>
{
	int animationIndex;
	SDL_Texture *texture;
public:
	SetAnimationMessage(int animationIndex, SDL_Texture *texture)
		: animationIndex(animationIndex), texture(texture)
	{
	}

	int getAnimationIndex() const { return animationIndex; }
	SDL_Texture *getTexture() const { return texture; }
};

class FrameChangeMessage : public Message<FrameChangeMessage>
{
	int frameNumber;

public:
	FrameChangeMessage(int frameNumber) : frameNumber(frameNumber) {}
	int getFrameNumber() const { return frameNumber; }
};

class SetTextureMessage : public Message<SetTextureMessage>
{
	SDL_Texture *texture;
public:
	SetTextureMessage(SDL_Texture *texture) : texture(texture) {}
	SDL_Texture *getTexture() const { return texture; }
};

class AddImpulseMessage : public Message<AddImpulseMessage>
{
	glm::vec2 impulse;
public:
	AddImpulseMessage(const glm::vec2 &impulse) : impulse(impulse) {}
	const glm::vec2 &getImpulse() const { return impulse; }
};

class AddForceMessage : public Message<AddForceMessage>
{
	glm::vec2 force;
public:
	AddForceMessage(const glm::vec2 &force) : force(force) {}
	const glm::vec2 &getForce() const { return force; }
};

class SetGroundedMessage : public Message<SetGroundedMessage>
{
	bool grounded;
public:
	SetGroundedMessage(bool grounded) : grounded(grounded) {}
	bool isGrounded() const { return grounded; }
};

enum class Axis : int
{
	X = 0,
	Y = 1,
};

class IntegrateVelocityMessage : public Message<IntegrateVelocityMessage>
{
public:
	IntegrateVelocityMessage(Axis axis, float deltaTime) : axis(axis), deltaTime(deltaTime) {}
	float getDeltaTime() const { return deltaTime; }
	Axis getAxis() const { return axis; }

private:
	Axis axis;
	float deltaTime;
};

class ScaleVelocityAxisMessage : public Message<ScaleVelocityAxisMessage>
{
	Axis axis;
	float factor;
public:
	ScaleVelocityAxisMessage(Axis axis, float factor) : axis(axis), factor(factor) {}
	Axis getAxis() const { return axis; }
	float getFactor() const { return factor; }
};

class FallingMessage : public Message<FallingMessage>
{
};

class JumpMessage : public Message<JumpMessage>
{
};

class CollisionMessage : public Message<CollisionMessage>
{
	GameObject &other;
	glm::vec2 overlap;
	glm::vec2 normal;

public:
	CollisionMessage(GameObject &other, const glm::vec2 &overlap, const glm::vec2 &normal) :
		other(other), overlap(overlap), normal(normal)
	{
	}

	const GameObject &getOther() const { return other; }
	const glm::vec2 &getOverlap() const { return overlap; }
	const glm::vec2 &getNormal() const { return normal; }
};

class VelocityMessage : public Message<VelocityMessage>
{
	glm::vec2 velocity;
public:
	VelocityMessage(const glm::vec2 &velocity) : velocity(velocity) {}
	const glm::vec2 &getVelocity() const { return velocity; }
};

class DirectionMessage : public Message<DirectionMessage>
{
	float direction;
public:
	DirectionMessage(float direction) : direction(direction) {}
	float getDirection() const { return direction; }
};

class ViewportMessage : public Message<ViewportMessage>
{
	glm::vec2 position;
	glm::vec2 size;

public:
	ViewportMessage(const glm::vec2 &position, const glm::vec2 &size) : position(position), size(size) {}
	glm::vec2 getPosition() const { return position; }
	glm::vec2 getSize() const { return size; }
};

class ShootStartMessage : public Message<ShootStartMessage>
{
};

class ShootEndMessage : public Message<ShootStartMessage>
{
};
