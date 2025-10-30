#pragma once

#include <components/component.h>

class DamageEvent;

class HealthComponent : public Component
{
	int hp;
public:
	HealthComponent(Node &owner, int hp);
	void onEvent(const DamageEvent &event);
};
