#include "playercontrolsystem.h"

#include <messaging/eventqueue.h>
#include <messaging/events.h>


PlayerControlSystem::PlayerControlSystem()
{
	EventQueue::get().dispatcher.registerHandler2<CollisionEvent>(this);
	EventQueue::get().dispatcher.registerHandler2<FallingEvent>(this);
	EventQueue::get().dispatcher.registerHandler2<JumpEvent>(this);
	EventQueue::get().dispatcher.registerHandler2<ShootBeginEvent>(this);
	EventQueue::get().dispatcher.registerHandler2<ShootEndEvent>(this);
}

void PlayerControlSystem::update(Node &node)
{
	//switch (currentState)
	//{
	//	case PState::idle:
	//	case PState::shooting:
	//	{
	//		// holding a direction, start running
	//		if (direction)
	//		{
	//			transitionState(!shooting ? PState::running : PState::runningShooting);
	//		}
	//		break;
	//	}
	//	case PState::running:
	//	case PState::runningShooting:
	//	{
	//		if (direction == 0)
	//		{
	//			// no longer holding direction, go to idle
	//			transitionState(!shooting ? PState::idle : PState::shooting);
	//		}
	//		else if (direction * velocity.x < 0)
	//		{
	//			// if direction we're holding is opposite to velocity, use sliding state
	//			transitionState(!shooting ? PState::sliding : PState::slidingShooting);
	//			slideTimer.reset();
	//		}
	//		break;
	//	}
	//	case PState::sliding:
	//	case PState::slidingShooting:
	//	{
	//		if (slideTimer.step(FrameContext::global().deltaTime))
	//		{
	//			if (direction == 0)
	//			{
	//				// if no longer holding direction, go to idle
	//				transitionState(!shooting ? PState::idle : PState::shooting);
	//			}
	//			else if (direction * velocity.x > 0)
	//			{
	//				// if direction and velocity directions match, go to running
	//				transitionState(!shooting ? PState::running : PState::runningShooting);
	//			}
	//		}
	//		break;
	//	}
	//}
}

void PlayerControlSystem::transitionState(Node &node, PState newState)
{
	auto [ic, pcc, pc] = getRequiredComponents(node);

	// on enter actions
	switch (newState)
	{
		case PState::idle:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getIdleAnimation(), pcc->getIdleTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::shooting:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getShootAnimation(), pcc->getShootTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::running:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getRunAnimation(), pcc->getRunTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::runningShooting:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getRunShootAnimation(), pcc->getRunShootTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::sliding:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getSlideAnimation(), pcc->getSlideTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::slidingShooting:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getSlideShootAnimation(), pcc->getSlideShootTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::airborne:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getJumpAnimation(), pcc->getJumpTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::airborneShooting:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getRunShootAnimation(), pcc->getRunShootTexture(), AnimationPlaybackMode::continuous);
			break;
		}
		case PState::falling:
		{
			EventQueue::get().enqueue2<AnimationPlayEvent>(node.getHandle(), 0, pcc->getJumpAnimation(), pcc->getJumpTexture(), AnimationPlaybackMode::continuous);
			break;
		}
	}
	pcc->setCurrentState(newState);
}

void PlayerControlSystem::onEvent(NodeHandle target, const CollisionEvent &event)
{
	Node &node = World::get().getNode(target);
	if (hasRequiredComponents(node))
	{
		auto [ic, pcc, pc] = getRequiredComponents(node);

		float dotY = glm::dot(event.getNormal(), glm::vec2(0, 1));
		float dotX = glm::dot(event.getNormal(), glm::vec2(1, 0));

		if (dotY == 1.0f)
		{
			// landed on something
			if (pcc->getCurrentState() == PState::airborne ||
				pcc->getCurrentState() == PState::airborneShooting)
			{
				transitionState(node, pc->getVelocity().x != 0 ?
					(!pcc->isShooting() ? PState::running : PState::runningShooting) :
					(!pcc->isShooting() ? PState::idle : PState::shooting));
			}
		}
	}
}

void PlayerControlSystem::onEvent(NodeHandle target, const FallingEvent &event)
{
	Node &node = World::get().getNode(target);
	auto [ic, pcc, pc] = getRequiredComponents(node);
	if (pcc->getCurrentState() != PState::airborne)
	{
		transitionState(node, !pcc->isShooting() ? PState::airborne : PState::airborneShooting);
	}
}

void PlayerControlSystem::onEvent(NodeHandle target, const JumpEvent &event)
{
	Node &node = World::get().getNode(target);
	auto [ic, pcc, pc] = getRequiredComponents(node);
	if (pcc->getCurrentState() != PState::airborne &&
		pcc->getCurrentState() != PState::airborneShooting)
	{
		transitionState(node, !pcc->isShooting() ? PState::airborne : PState::airborneShooting);
		pc->addImpulse(glm::vec2(0, -250.0f));
	}
}

void PlayerControlSystem::onEvent(NodeHandle target, const ShootBeginEvent &event)
{
	Node &node = World::get().getNode(target);
	auto [ic, pcc, pc] = getRequiredComponents(node);

	pcc->setIsShooting(true);
	switch (pcc->getCurrentState())
	{
		case PState::idle:
		{
			transitionState(node, PState::shooting);
			break;
		}
		case PState::running:
		{
			transitionState(node, PState::runningShooting);
			break;
		}
		case PState::airborne:
		{
			transitionState(node, PState::airborneShooting);
			break;
		}
		case PState::sliding:
		{
			transitionState(node, PState::slidingShooting);
			break;
		}
	}
}
void PlayerControlSystem::onEvent(NodeHandle target, const ShootEndEvent &event)
{
	Node &node = World::get().getNode(target);
	auto [ic, pcc, pc] = getRequiredComponents(node);

	pcc->setIsShooting(false);
	switch (pcc->getCurrentState())
	{
		case PState::shooting:
		{
			transitionState(node, PState::idle);
			break;
		}
		case PState::runningShooting:
		{
			transitionState(node, PState::running);
			break;
		}
		case PState::airborneShooting:
		{
			transitionState(node, PState::airborne);
			break;
		}
		case PState::slidingShooting:
		{
			transitionState(node, PState::slidingShooting);
		}
	}
}
