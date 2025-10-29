#include <messaging/event.h>

class DamageEvent : public Event<DamageEvent>
{
	int amount;
public:
	DamageEvent(int amount) : amount(amount) {}

	int getAmount() const { return amount; }
};