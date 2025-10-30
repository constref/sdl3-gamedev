#include "enemycomponent.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <logger.h>

#include "events.h"

void EnemyComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<DeathEvent>(this);
	eventDispatcher.registerHandler<AnimationEndEvent>(this);
}

void EnemyComponent::onDetached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) const
{
}

void EnemyComponent::onEvent(const DeathEvent &event)
{
	const Resources &res = Resources::get();
	EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), ComponentStage::PostRender);
	if (type == EnemyType::creeper)
	{
		owner.pushData(SetAnimationCommand{ res.ANIM_ENEMY_DIE, res.texEnemyDie, true });
	}
}

void EnemyComponent::onEvent(const AnimationEndEvent &event)
{
	const Resources &res = Resources::get();

	if (type == EnemyType::creeper)
	{
		if (event.getIndex() == res.ANIM_ENEMY_DIE)
		{
			EventQueue::get().enqueue<AnimationStopEvent>(owner.getHandle(), ComponentStage::Animation);
		}
	}
}
