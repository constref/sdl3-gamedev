#include "healthcomponent.h"
#include "events.h"
#include <logger.h>
#include <messaging/eventdispatcher.h>
#include <messaging/eventqueue.h>

void HealthComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<DamageEvent>(this);
}

void HealthComponent::onEvent(const DamageEvent &event)
{
	if (hp > 0)
	{
		int oldHp = hp;
		hp -= event.getAmount();
		Logger::info(this, std::format("{} - {} = {}", oldHp, event.getAmount(), hp));

		if (hp <= 0)
		{
			hp = 0;
			EventQueue::get().enqueue<DeathEvent>(owner.getHandle(), ComponentStage::Gameplay);
		}
	}
}

