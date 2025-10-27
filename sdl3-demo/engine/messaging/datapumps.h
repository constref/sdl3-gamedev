#pragma once

#include <glm/glm.hpp>
#include "datapump.h"

struct SDL_Texture;

class SetAnimationDPump : public DataPump<SetAnimationDPump>
{
	int animationIndex;
	SDL_Texture *texture;
public:
	SetAnimationDPump(int animationIndex, SDL_Texture *texture)
		: animationIndex(animationIndex), texture(texture)
	{
	}

	int getAnimationIndex() const { return animationIndex; }
	SDL_Texture *getTexture() const { return texture; }
};

class FrameChangeDPump : public DataPump<FrameChangeDPump>
{
	int frameNumber;

public:
	FrameChangeDPump(int frameNumber) : frameNumber(frameNumber) {}
	int getFrameNumber() const { return frameNumber; }
};

class SetTextureDPump : public DataPump<SetTextureDPump>
{
	SDL_Texture *texture;
public:
	SetTextureDPump(SDL_Texture *texture) : texture(texture) {}
	SDL_Texture *getTexture() const { return texture; }
};

class AddImpulseDPump : public DataPump<AddImpulseDPump>
{
	glm::vec2 impulse;
public:
	AddImpulseDPump(const glm::vec2 &impulse) : impulse(impulse) {}
	const glm::vec2 &getImpulse() const { return impulse; }
};

class AddForceDPump : public DataPump<AddForceDPump>
{
	glm::vec2 force;
public:
	AddForceDPump(const glm::vec2 &force) : force(force) {}
	const glm::vec2 &getForce() const { return force; }
};

class SetGroundedDPump : public DataPump<SetGroundedDPump>
{
	bool grounded;
public:
	SetGroundedDPump(bool grounded) : grounded(grounded) {}
	bool isGrounded() const { return grounded; }
};

enum class Axis : int
{
	X = 0,
	Y = 1,
};

class ScaleVelocityAxisDPump : public DataPump<ScaleVelocityAxisDPump>
{
	Axis axis;
	float factor;
public:
	ScaleVelocityAxisDPump(Axis axis, float factor) : axis(axis), factor(factor) {}
	Axis getAxis() const { return axis; }
	float getFactor() const { return factor; }
};

class TentativeVelocityDPump : public DataPump<TentativeVelocityDPump>
{
	glm::vec2 delta;

public:
	TentativeVelocityDPump(glm::vec2 delta)
	{
		this->delta = delta;
	}

	glm::vec2 getDelta() const { return delta; }
};

class VelocityDPump : public DataPump<VelocityDPump>
{
	glm::vec2 velocity;
public:
	VelocityDPump(const glm::vec2 &velocity) : velocity(velocity) {}
	const glm::vec2 &getVelocity() const { return velocity; }
};

class DirectionDPump : public DataPump<DirectionDPump>
{
	float direction;
public:
	DirectionDPump(float direction) : direction(direction) {}
	float getDirection() const { return direction; }
};

class ViewportDPump : public DataPump<ViewportDPump>
{
	glm::vec2 position;
	glm::vec2 size;

public:
	ViewportDPump(const glm::vec2 &position, const glm::vec2 &size) : position(position), size(size) {}
	glm::vec2 getPosition() const { return position; }
	glm::vec2 getSize() const { return size; }
};
