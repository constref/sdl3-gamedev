#include "enemysystem.h"

#include <messaging/messaging.h>
#include <resources.h>
#include "../events.h"

EnemySystem::EnemySystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<DeathEvent>(this);
}

void EnemySystem::onEvent(NodeHandle target, const DeathEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [ec, pc, cc] = getRequiredComponents(node);

		node.removeComponent<CollisionComponent>();
		if (ec->getType() == EnemyType::creeper)
		{
			const Resources &res = Resources::get();
			ec->setState(EnemyState::dead);
			services.eventQueue().enqueue<AnimationPlayEvent>(node.getHandle(), 0, res.ANIM_ENEMY_DIE, res.texEnemyDie);
		}
	}
}
