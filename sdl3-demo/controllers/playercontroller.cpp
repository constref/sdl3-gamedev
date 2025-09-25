#include "playercontroller.h"
#include "../framecontext.h"
#include "../resources.h"
#include "../gameobject.h"
#include "../events.h"

#include "../components/animationcomponent.h"
#include "../components/physicscomponent.h"
#include "../components/rendercomponent.h"
#include "states/idlestate.h"

PlayerController::PlayerController(GameObject &owner) : Controller(owner)
{
	changeState(IdleState::instance());
}

void PlayerController::eventHandler(const FrameContext &ctx, int eventId)
{
	Controller::eventHandler(ctx, eventId);
	//switch (eventId)
	//{
	//	case static_cast<int>(Events::idle):
	//	{
	//		auto *animComp = owner.getComponent<AnimationComponent>();
	//		animComp->setAnimation(ctx.res.ANIM_PLAYER_IDLE);
	//		auto *renderComp = owner.getComponent<RenderComponent>();
	//		renderComp->setTexture(ctx.res.texIdle);
	//		break;
	//	}
	//	case static_cast<int>(Events::run):
	//	{
	//		auto *animComp = owner.getComponent<AnimationComponent>();
	//		animComp->setAnimation(ctx.res.ANIM_PLAYER_RUN);
	//		auto *renderComp = owner.getComponent<RenderComponent>();
	//		renderComp->setTexture(ctx.res.texRun);
	//		break;
	//	}
	//	case static_cast<int>(Events::landed):
	//	{
	//		auto *physComp = owner.getComponent<PhysicsComponent>();
	//		glm::vec2 velocity = physComp->getVelocity();
	//		velocity.y = 0;
	//		physComp->setVelocity(velocity);
	//		physComp->setGrounded(true);
	//		break;
	//	}
	//	case static_cast<int>(Events::jump):
	//	{
	//		auto *physComp = owner.getComponent<PhysicsComponent>();
	//		if (physComp)
	//		{
	//			const float JUMP_FORCE = -200.0f;
	//			if (physComp->isGrounded())
	//			{
	//				auto *animComp = owner.getComponent<AnimationComponent>();
	//				animComp->setAnimation(ctx.res.ANIM_PLAYER_RUN);
	//				auto *renderComp = owner.getComponent<RenderComponent>();
	//				renderComp->setTexture(ctx.res.texRun);

	//				physComp->setGrounded(false);
	//				physComp->setVelocity(physComp->getVelocity() + glm::vec2(0, -200));
	//			}
	//		}
	//		break;
	//	}
	//}
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
