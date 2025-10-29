#pragma once

#include <glm/glm.hpp>
#include "command.h"

struct SDL_Texture;

class SetAnimationCommand : public Command<SetAnimationCommand>
{
	int animationIndex;
	SDL_Texture *texture;
	bool notifyEnd;

public:
	SetAnimationCommand(int animationIndex, SDL_Texture *texture, bool notifEnd = false)
	{
		this->animationIndex = animationIndex;
		this->texture = texture;
		this->notifyEnd = notifyEnd;
	}

	int getAnimationIndex() const { return animationIndex; }
	SDL_Texture *getTexture() const { return texture; }
	bool shouldNotifyEnd() const { return notifyEnd; }
};

class FrameChangeCommand : public Command<FrameChangeCommand>
{
	int frameNumber;

public:
	FrameChangeCommand(int frameNumber) : frameNumber(frameNumber) {}
	int getFrameNumber() const { return frameNumber; }
};

class SetTextureCommand : public Command<SetTextureCommand>
{
	SDL_Texture *texture;
public:
	SetTextureCommand(SDL_Texture *texture) : texture(texture) {}
	SDL_Texture *getTexture() const { return texture; }
};

class AddImpulseCommand : public Command<AddImpulseCommand>
{
	glm::vec2 impulse;
public:
	AddImpulseCommand(const glm::vec2 &impulse) : impulse(impulse) {}
	const glm::vec2 &getImpulse() const { return impulse; }
};

class AddForceCommand : public Command<AddForceCommand>
{
	glm::vec2 force;
public:
	AddForceCommand(const glm::vec2 &force) : force(force) {}
	const glm::vec2 &getForce() const { return force; }
};

enum class Axis : int
{
	X = 0,
	Y = 1,
};

class ScaleVelocityAxisCommand : public Command<ScaleVelocityAxisCommand>
{
	Axis axis;
	float factor;
public:
	ScaleVelocityAxisCommand(Axis axis, float factor) : axis(axis), factor(factor) {}
	Axis getAxis() const { return axis; }
	float getFactor() const { return factor; }
};

class TentativeVelocityCommand : public Command<TentativeVelocityCommand>
{
	glm::vec2 delta;

public:
	TentativeVelocityCommand(glm::vec2 delta)
	{
		this->delta = delta;
	}

	glm::vec2 getDelta() const { return delta; }
};

class UpdateVelocityCommand : public Command<UpdateVelocityCommand>
{
	glm::vec2 velocity;
public:
	UpdateVelocityCommand(const glm::vec2 &velocity) : velocity(velocity) {}
	const glm::vec2 &getVelocity() const { return velocity; }
};

class UpdateDirectionCommand : public Command<UpdateDirectionCommand>
{
	float direction;
public:
	UpdateDirectionCommand(float direction) : direction(direction) {}
	float getDirection() const { return direction; }
};

class UpdateViewportCommand : public Command<UpdateViewportCommand>
{
	glm::vec2 position;
	glm::vec2 size;

public:
	UpdateViewportCommand(const glm::vec2 &position, const glm::vec2 &size) : position(position), size(size) {}
	glm::vec2 getPosition() const { return position; }
	glm::vec2 getSize() const { return size; }
};
