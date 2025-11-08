#include "enemycomponent.h"
#include "enemycomponent.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <logger.h>

#include "events.h"

EnemyComponent::EnemyComponent(Node &owner, EnemyType type) : Component(owner, FrameStage::Gameplay), type(type)
{
	state = EnemyState::idle;
	dmgTime = 0;

	owner.getEventDispatcher().registerHandler<DamageEvent>(this);
	owner.getEventDispatcher().registerHandler<DeathEvent>(this);
}

void EnemyComponent::onEvent(const DeathEvent &event)
{
	const Resources &res = Resources::get();
	EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), 0);

	if (type == EnemyType::creeper)
	{
		EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY_DIE, res.texEnemyDie);
		EventQueue::get().enqueue<AnimationStopEvent>(owner.getHandle(), res.enemyAnims[res.ANIM_ENEMY_DIE].getLength());
	}
}

void EnemyComponent::onEvent(const DamageEvent &event)
{
	const Resources &res = Resources::get();
	if (type == EnemyType::creeper)
	{
		switch (state)
		{
			case EnemyState::idle:
			{
				state = EnemyState::damaged;
				EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY_HIT, res.texEnemyHit, AnimationPlaybackMode::continuous);
				dmgTime = FrameContext::global().globalTime;
				break;
			}
			case EnemyState::damaged:
			{
			}
		}
	}
}
