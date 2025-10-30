#pragma once

#include <components/component.h>

class DeathEvent;
class AnimationEndEvent;

enum class EnemyType
{
	creeper
};

class EnemyComponent : public Component
{
	const EnemyType type;
public:
	EnemyComponent(Node &owner, EnemyType type);

	void onEvent(const DeathEvent &event);
	void onEvent(const AnimationEndEvent &event);
};