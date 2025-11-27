#include "damagesystem.h"
#include "../events.h"

DamageSystem::DamageSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<DamageEvent>(this);
}

void DamageSystem::update(Node &node)
{
	//const Resources &res = Resources::get();
	//switch (state)
	//{
	//	case EnemyState::damaged:
	//	{
	//		removeTimer(timerDamaged);
	//		state = EnemyState::idle;
	//		services.eventQueue().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY, res.texEnemy, AnimationPlaybackMode::continuous);

	//		// turn towards damage source
	//		Node &sourceNode = services.world().getNode(damageSource);
	//		glm::vec2 sourceDir = glm::normalize(glm::vec2(sourceNode.getPosition().x - owner.getPosition().x, 0));
	//		owner.getCommandDispatcher().send<UpdateDirectionCommand>(UpdateDirectionCommand(sourceDir.x));
	//		break;
	//	}
	//}
}

void DamageSystem::onEvent(NodeHandle target, const DamageEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [hc, pc, sc] = getRequiredComponents(node);
		pc->addImpulse(glm::vec2(200, 0));

		hc->hp -= event.getAmount();
		if (hc->hp <= 0)
		{
			hc->hp = 0;
			services.eventQueue().enqueue<DeathEvent>(target, 0);
		}

		sc->setShouldFlash(true);
	}
}
