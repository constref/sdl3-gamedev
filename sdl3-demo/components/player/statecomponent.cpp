#include "statecomponent.h"

#include "../resources.h"
#include "../gameobject.h"
#include "../commands.h"
#include "../events.h"
#include "../../coresubjects.h"

StateComponent::StateComponent(GameObject &owner) : Component(owner)
{
	direction = 0;
	velocity = glm::vec2(0);
	currentState = PState::idle;
	idleAnimationIndex = 0;
	idleTexture = nullptr;
	runAnimationIndex = 0;
	runTexture = nullptr;
	slideAnimationIndex = 0;
	slideTexture = nullptr;
}

void StateComponent::registerObservers(SubjectRegistry &registry)
{
	registry.addObserver<float>(CoreSubjects::DIRECTION, [this](const float &direction) {
		this->direction = direction;
	});
	registry.addObserver<glm::vec2>(CoreSubjects::VELOCITY, [this](const glm::vec2 &velocity) {
		this->velocity = velocity;
	});
}

void StateComponent::update(const FrameContext &ctx)
{
}

