#include "enemysystem.h"

#include <messaging/messaging.h>
#include <componentsystems.h>
#include <resources.h>
#include "../events.h"

EnemySystem::EnemySystem(Services &services, int animEnemyDeath) : System(services)
{
	this->animEnemyDeath = animEnemyDeath;
	services.eventQueue().dispatcher.registerHandler<DeathEvent>(this);
}

void EnemySystem::onEvent(NodeHandle target, const DeathEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [ec, pc, cc] = getRequiredComponents(node);

		services.compSys().removeComponent(node, *cc);
		if (ec->getType() == EnemyType::creeper)
		{
			const Resources &res = Resources::get();
			ec->setState(EnemyState::dead);
			services.eventQueue().enqueue<AnimationPlayEvent>(node.getHandle(), 0, animEnemyDeath, res.texEnemyDie);
		}
	}
}
