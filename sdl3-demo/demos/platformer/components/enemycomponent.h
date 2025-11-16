#pragma once

#include <components/component.h>
#include <timer.h>
#include <nodehandle.h>

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
	Timer damagedTimer;
	NodeHandle damageSource;

public:
	EnemyComponent(Node &owner, EnemyType type);

	EnemyType getType() const { return type; }
	EnemyState getState() const { return state; }
	void setState(EnemyState state) { this->state = state; }
	Timer &getDamagedTimer() { return damagedTimer; }
};