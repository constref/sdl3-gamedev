#pragma once

#include <components/component.h>
#include <timer.h>
#include <nodehandle.h>

class DamageEvent;
class DeathEvent;
class TimerOnTimeout;

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

	void onEvent(const DamageEvent &event);
	void onEvent(const DeathEvent &event);
	void onEvent(const TimerOnTimeout &event);
};