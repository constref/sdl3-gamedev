#include "healthcomponent.h"
#include "events.h"
#include <logger.h>
#include <messaging/eventdispatcher.h>
#include <messaging/eventqueue.h>

HealthComponent::HealthComponent(Node &owner, int hp) : Component(owner, ComponentStage::Gameplay)
{
	this->hp = hp;
	owner.getEventDispatcher().registerHandler<DamageEvent>(this);
}

void HealthComponent::onEvent(const DamageEvent &event)
{
	if (hp > 0)
	{
		int oldHp = hp;
		hp -= event.getAmount();

		if (hp <= 0)
		{
			hp = 0;
			EventQueue::get().enqueue<DeathEvent>(owner.getHandle(), ComponentStage::Gameplay);
		}
	}
}

