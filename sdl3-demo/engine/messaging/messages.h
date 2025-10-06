#pragma once

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

class JumpMessage : public Message<JumpMessage>
{
};


enum class Commands : int
{
	LandOnGround,
};