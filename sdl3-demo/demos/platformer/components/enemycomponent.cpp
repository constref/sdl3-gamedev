#include "enemycomponent.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <logger.h>
#include <node.h>
#include <glm/glm.hpp>

#include "../events.h"

EnemyComponent::EnemyComponent(Node &owner, EnemyType type)
	: Component(owner, FrameStage::Gameplay), type(type), timerDamaged(0.5f)
{
	state = EnemyState::idle;

	owner.getEventDispatcher().registerHandler<DamageEvent>(this);
	owner.getEventDispatcher().registerHandler<DeathEvent>(this);
	owner.getEventDispatcher().registerHandler<TimerOnTimeout>(this);
}

void EnemyComponent::onEvent(const DamageEvent &event)
{
	const Resources &res = Resources::get();
	if (type == EnemyType::creeper)
	{
		timerDamaged.reset();
		addTimer(timerDamaged);

		damageSource = event.getSource();
		Node &sourceNode = World::get().getNode(damageSource);
		glm::vec2 sourceDir = glm::normalize(glm::vec2(sourceNode.getPosition().x - owner.getPosition().x, 0));
		owner.getCommandDispatcher().send<UpdateDirectionCommand>(UpdateDirectionCommand(sourceDir.x));

		// apply push force from projectile
		owner.sendCommand<AddImpulseCommand>(AddImpulseCommand{ glm::vec2(200 * -sourceDir.x, 220.0f) });

		switch (state)
		{
			case EnemyState::idle:
			{
				state = EnemyState::damaged;
				EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY_HIT, res.texEnemyHit, AnimationPlaybackMode::continuous);
				break;
			}
		}
	}
}

void EnemyComponent::onEvent(const TimerOnTimeout &event)
{
	const Resources &res = Resources::get();
	switch (state)
	{
		case EnemyState::damaged:
		{
			removeTimer(timerDamaged);
			state = EnemyState::idle;
			EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY, res.texEnemy, AnimationPlaybackMode::continuous);

			// turn towards damage source
			Node &sourceNode = World::get().getNode(damageSource);
			glm::vec2 sourceDir = glm::normalize(glm::vec2(sourceNode.getPosition().x - owner.getPosition().x, 0));
			owner.getCommandDispatcher().send<UpdateDirectionCommand>(UpdateDirectionCommand(sourceDir.x));
			break;
		}
	}
}

void EnemyComponent::onEvent(const DeathEvent &event)
{
	const Resources &res = Resources::get();
	EventQueue::get().enqueue<RemoveColliderEvent>(owner.getHandle(), 0);

	if (type == EnemyType::creeper)
	{
		state = EnemyState::dead;
		EventQueue::get().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY_DIE, res.texEnemyDie);
		EventQueue::get().enqueue<AnimationStopEvent>(owner.getHandle(), res.enemyAnims[res.ANIM_ENEMY_DIE].getLength());
	}
}

