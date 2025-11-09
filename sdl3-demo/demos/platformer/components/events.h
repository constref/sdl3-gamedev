#pragma once

#include <messaging/event.h>
#include <nodehandle.h>

class DamageEvent : public Event<DamageEvent>
{
	NodeHandle source;
	int amount;

public:
	DamageEvent(NodeHandle source, int amount) : source(source), amount(amount) {}

	int getAmount() const { return amount; }
	NodeHandle getSource() const { return source; }
};

class DeathEvent : public  Event<DeathEvent> { };