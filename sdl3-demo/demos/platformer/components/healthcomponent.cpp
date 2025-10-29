#include "healthcomponent.h"
#include "events.h"
#include <logger.h>
#include <messaging/eventdispatcher.h>

void HealthComponent::onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	eventDispatcher.registerHandler<HealthComponent, DamageEvent>(this);
}

void HealthComponent::onEvent(const DamageEvent &event)
{
	int oldHp = hp;
	hp -= event.getAmount();

	Logger::info(this, std::format("{} - {} = {}", oldHp, event.getAmount(), hp));
}

