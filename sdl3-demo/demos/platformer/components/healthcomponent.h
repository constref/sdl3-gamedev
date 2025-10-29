#pragma once

#include <components/component.h>

class DamageEvent;

class HealthComponent : public Component
{
public:
	int hp;

	HealthComponent(Node &owner, int hp) : Component(owner, ComponentStage::Gameplay)
	{
		this->hp = hp;
	}
	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher);

	void onEvent(const DamageEvent &event);
};
