#pragma once

#include <glm/glm.hpp>
#include "../component.h"

struct SDL_Texture;
class JumpMessage;
class CollisionMessage;

enum class PState
{
	idle,
	running,
	airborne,
	falling,
	sliding
};

class PlayerControllerComponent : public Component
{
	float direction;
	glm::vec2 velocity;

	PState currentState;
	int idleAnimationIndex;
	SDL_Texture *idleTexture;
	int runAnimationIndex;
	SDL_Texture *runTexture;
	int slideAnimationIndex;
	SDL_Texture *slideTexture;

public:
	PlayerControllerComponent(GameObject &owner);
	void update(const FrameContext &ctx) override;
	void onAttached(SubjectRegistry &registry, MessageDispatch &msgDispatch) override;
	void onStart() override;
	void registerObservers(SubjectRegistry &registry) override;
	void transitionState(PState newState);
	void onEvent(int eventId) override;
	void onMessage(const JumpMessage &msg);
	void onMessage(const CollisionMessage &msg);

	void setIdleAnimation(int index) { idleAnimationIndex = index; }
	void setIdleTexture(SDL_Texture *tex) { idleTexture = tex; }
	void setRunAnimation(int index) { runAnimationIndex = index; }
	void setRunTexture(SDL_Texture *tex) { runTexture = tex; }
	void setSlideAnimation(int index) { slideAnimationIndex = index; }
	void setSlideTexture(SDL_Texture *tex) { slideTexture = tex; }
};