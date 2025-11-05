#include "enemycomponent.h"
#include "enemycomponent.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <logger.h>

#include "events.h"

EnemyComponent::EnemyComponent(Node &owner, EnemyType type) : Component(owner, ComponentStage::Gameplay), type(type)
{
	owner.getEventDispatcher().registerHandler<DeathEvent>(this);
	owner.getEventDispatcher().registerHandler<AnimationEndEvent>(this);
}

void EnemyComponent::onEvent(const DeathEvent &event)
{
	const Resources &res = Resources::get();
	EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), ComponentStage::PostRender);
	if (type == EnemyType::creeper)
	{
		//owner.sendCommand(SetAnimationCommand{ res.ANIM_ENEMY_DIE, res.texEnemyDie, true });
		EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, res.ANIM_ENEMY_DIE, res.texEnemyDie);
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
