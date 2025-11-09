#pragma once

#include <components/component.h>
#include <timer.h>
#include <nodehandle.h>

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
	Timer timerDamaged;
	NodeHandle damageSource;

public:
	EnemyComponent(Node &owner, EnemyType type);

	void update();
	void onEvent(const DamageEvent &event);
	void onEvent(const DeathEvent &event);
};