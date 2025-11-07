#include "enemycomponent.h"
#include "enemycomponent.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <logger.h>

#include "events.h"

EnemyComponent::EnemyComponent(Node &owner, EnemyType type) : Component(owner, ComponentStage::Gameplay), type(type)
{
	state = EnemyState::idle;

	owner.getEventDispatcher().registerHandler<DamageEvent>(this);
	owner.getEventDispatcher().registerHandler<DeathEvent>(this);
}

void EnemyComponent::onEvent(const DeathEvent &event)
{
	const Resources &res = Resources::get();
	EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), ComponentStage::PostRender, 0);

	if (type == EnemyType::creeper)
	{
		EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, res.ANIM_ENEMY_DIE, res.texEnemyDie);
		EventQueue::get().enqueue<AnimationStopEvent>(owner.getHandle(), ComponentStage::Animation, res.enemyAnims[res.ANIM_ENEMY_DIE].getLength());
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
				EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 0, res.ANIM_ENEMY_HIT, res.texEnemyHit, AnimationPlaybackMode::continuous);
				EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), ComponentStage::Animation, 1.0f, res.ANIM_ENEMY, res.texEnemy, AnimationPlaybackMode::continuous);
				break;
			}
		}
	}
}
