#pragma once

#include <components/component.h>

class HealthComponent : public Component
{
public:
	int hp;
	HealthComponent(Node &owner, int hp);
};
