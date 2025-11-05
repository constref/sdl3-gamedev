#pragma once

#include <messaging/event.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <nodehandle.h>

struct SDL_Texture;

struct KeyboardEvent : public Event<KeyboardEvent>
{
	enum class State
	{
		down,
		up
	};

	SDL_Scancode scancode;
	State state;

	KeyboardEvent(SDL_Scancode scancode, State state) :
		scancode(scancode), state(state)
	{
	}
};

class CollisionEvent : public Event<CollisionEvent>
{
	NodeHandle other;
	glm::vec2 overlap;
	glm::vec2 normal;

public:
	CollisionEvent(NodeHandle other, const glm::vec2 &overlap, const glm::vec2 &normal) :
		other(other), overlap(overlap), normal(normal)
	{
	}

	NodeHandle getOther() const { return other; }
	const glm::vec2 &getOverlap() const { return overlap; }
	const glm::vec2 &getNormal() const { return normal; }
};

class AnimationPlayEvent : public Event<AnimationPlayEvent>
{
	int animationIndex;
	SDL_Texture *texture;
	bool oneShot = true;

public:
	AnimationPlayEvent(int animationIndex, SDL_Texture *texture, bool oneShot = true)
	{
		this->animationIndex = animationIndex;
		this->texture = texture;
		this->oneShot = true;
	}

	int getAnimationIndex() const { return animationIndex; }
	SDL_Texture *getTexture() const { return texture; }
	bool isOneShot() const { return oneShot; }
};

class AnimationEndEvent : public Event<AnimationEndEvent>
{
	int index;

public:
	AnimationEndEvent(int index)
	{
		this->index = index;
	}

	int getIndex() const { return index; }
};

class AnimationStopEvent : public Event<AnimationStopEvent> { };
class RemoveColliderEvent : public Event<RemoveColliderEvent> { };
class NodeRemovalEvent : public Event<NodeRemovalEvent> {};
class FallingEvent : public Event<FallingEvent> {};
class JumpEvent : public Event<JumpEvent> {};
class ShootBeginEvent : public Event<ShootBeginEvent> {};
class ShootEndEvent : public Event<ShootEndEvent> {};
