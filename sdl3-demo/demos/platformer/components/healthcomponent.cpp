#include "healthcomponent.h"
#include "events.h"
#include <logger.h>
#include <messaging/messaging.h>

HealthComponent::HealthComponent(Node &owner, int hp) : Component(owner, FrameStage::Gameplay)
{
	this->hp = hp;
	owner.getEventDispatcher().registerHandler<DamageEvent>(this);
}

void HealthComponent::onEvent(const DamageEvent &event)
{
	if (hp > 0)
	{
		hp -= event.getAmount();
		if (hp <= 0)
		{
			hp = 0;
			EventQueue::get().enqueue<DeathEvent>(owner.getHandle(), 0);
		}
	}
}
