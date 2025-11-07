#pragma once

#include <components/component.h>

class DamageEvent;
class DeathEvent;

enum class EnemyState
{
	idle,
	damaged,
	dead
};

enum class EnemyType
{
	creeper
};

class EnemyComponent : public Component
{
	EnemyState state;
	const EnemyType type;

public:
	EnemyComponent(Node &owner, EnemyType type);

	void onEvent(const DamageEvent &event);
	void onEvent(const DeathEvent &event);
};