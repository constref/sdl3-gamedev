#pragma once

#include <messaging/event.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <nodehandle.h>
#include <animationplaybackmode.h>

struct SDL_Texture;

class KeyUpEvent : public Event<KeyUpEvent, FrameStage::Input>
{
public:
	const SDL_Scancode scancode;
	KeyUpEvent(SDL_Scancode scancode) : scancode(scancode) { }
};

class KeyDownEvent : public Event<KeyDownEvent, FrameStage::Input>
{
public:
	const SDL_Scancode scancode;
	KeyDownEvent(SDL_Scancode scancode) : scancode(scancode) { }
};

class ComponentRemovalEvent : public Event<ComponentRemovalEvent, FrameStage::End>
{
	NodeHandle target;
	Component *component;
public:
	ComponentRemovalEvent(NodeHandle target, Component *component) : target(target), component(component) {}
};

class DirectionChangedEvent : public Event<DirectionChangedEvent, FrameStage::Physics>
{
	glm::vec2 direction;
public:
	DirectionChangedEvent(glm::vec2 direction) : direction(direction) {}

	glm::vec2 getDirection() const { return direction; }
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

template<typename CallbackEvent>
class AddTimerEvent : public Event<AddTimerEvent<CallbackEvent>, FrameStage::Start>
{
	float duration;
	NodeHandle target;

public:
	AddTimerEvent(float duration, NodeHandle target)
	{
		this->duration = duration;
		this->target = target;
	}
};

class TimerTimeoutEvent : public Event<TimerTimeoutEvent, FrameStage::Gameplay>
{
	int timeouts;
public:
	TimerTimeoutEvent(int timeouts)
	{
		this->timeouts = timeouts;
	}

	int getTimeouts() const { return timeouts; }
};

class AnimationStopEvent : public Event<AnimationStopEvent, FrameStage::Animation> { };
class NodeRemovalEvent : public Event<NodeRemovalEvent, FrameStage::End> {};
class FallingEvent : public Event<FallingEvent, FrameStage::Physics> {};
class JumpEvent : public Event<JumpEvent, FrameStage::Gameplay> {};
class ShootBeginEvent : public Event<ShootBeginEvent, FrameStage::Gameplay> {};
class ShootEndEvent : public Event<ShootEndEvent, FrameStage::Gameplay> {};
