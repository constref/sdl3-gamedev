#pragma once

#include <cassert>

constexpr static int MAX_COMMANDS = 1000;

class CommandBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename CommandType>
class Command : public CommandBase
{

public:
	constexpr static int index()
	{
		assert(nextIndex < MAX_COMMANDS && "Exceeded maximum command index limit");
		static int commandIndex = nextIndex++;
		return commandIndex;
	}

	template<typename Comp>
	void visit(Comp &component)
	{
		if constexpr (requires { component.onCommand(std::declval<const CommandType &>()); })
		{
			component.onCommand(static_cast<const CommandType &>(*this));
		}
	}
};

struct SDL_Texture;

class SetAnimationCommand : public Command<SetAnimationCommand>
{
	int animationIndex;
	SDL_Texture *texture;
public:
	SetAnimationCommand(int animationIndex, SDL_Texture *texture)
		: animationIndex(animationIndex), texture(texture)
	{
	}

	int getAnimationIndex() const { return animationIndex; }
	SDL_Texture *getTexture() const { return texture; }
};

enum class Commands : int
{
	SetAnimation,
	SetTexture,
	AddImpulse,
	AddForce,
	LandOnGround,
	SetGrounded,
	IntegrateVelocityX,
	IntegrateVelocityY,
	ZeroVelocityX,
	ZeroVelocityY,
	Jump
};