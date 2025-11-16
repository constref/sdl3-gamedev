#pragma once

#include <components/component.h>

class DamageEvent;

class HealthComponent : public Component
{
public:
	int hp;
	HealthComponent(Node &owner, int hp);
	void onEvent(const DamageEvent &event);
};
