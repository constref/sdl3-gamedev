#pragma once

#include <messaging/event.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <nodehandle.h>
#include <animationplaybackmode.h>

struct SDL_Texture;

struct KeyboardEvent : public Event<KeyboardEvent, FrameStage::Input>
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

class CollisionEvent : public Event<CollisionEvent, FrameStage::Physics>
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

class AnimationPlayEvent : public Event<AnimationPlayEvent, FrameStage::Animation>
{
	int animationIndex;
	SDL_Texture *texture;
	AnimationPlaybackMode mode;

public:
	AnimationPlayEvent(int animationIndex, SDL_Texture *texture, AnimationPlaybackMode mode = AnimationPlaybackMode::oneShot)
	{
		this->animationIndex = animationIndex;
		this->texture = texture;
		this->mode = mode;
	}

	int getAnimationIndex() const { return animationIndex; }
	SDL_Texture *getTexture() const { return texture; }
	AnimationPlaybackMode getPlaybackMode() const { return mode; }
};

class TimerOnTimeout : public Event<TimerOnTimeout, FrameStage::Gameplay>
{
	int tag;
public:
	TimerOnTimeout(int tag = 0)
	{
		this->tag = tag;
	}

	int getTag() const { return tag; }
};

class AnimationStopEvent : public Event<AnimationStopEvent, FrameStage::Animation> { };
class RemoveColliderEvent : public Event<RemoveColliderEvent, FrameStage::End> { };
class NodeRemovalEvent : public Event<NodeRemovalEvent, FrameStage::End> {};
class FallingEvent : public Event<FallingEvent, FrameStage::Physics> {};
class JumpEvent : public Event<JumpEvent, FrameStage::Gameplay> {};
class ShootBeginEvent : public Event<ShootBeginEvent, FrameStage::Gameplay> {};
class ShootEndEvent : public Event<ShootEndEvent, FrameStage::Gameplay> {};
