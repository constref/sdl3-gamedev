#include "playercontroller.h"
#include "../framecontext.h"
#include "../resources.h"
#include "../gameobject.h"
#include "../events.h"

#include "states/idlestate.h"

#include "../components/physicscomponent.h"

PlayerController::PlayerController(GameObject &owner) : Controller(owner)
{
	changeState(IdleState::instance());
}

void PlayerController::eventHandler(const FrameContext &ctx, int eventId)
{
	Controller::eventHandler(ctx, eventId);
}

void PlayerController::collisionHandler(GameObject &other, glm::vec2 overlap)
{
	auto *physicsComponent = owner.getComponent<PhysicsComponent>();
	if (physicsComponent)
	{
		glm::vec2 vel = physicsComponent->getVelocity();
		if (overlap.x < overlap.y)
		{
			vel.x = 0;
		}
		else
		{
			vel.y = 0;
		}
		physicsComponent->setVelocity(vel);
	}
}
