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
	EnemyComponent(Node &owner, EnemyType type)
		: Component(owner, ComponentStage::Gameplay), type(type) { }

	void onAttached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) override;
	void onDetached(CommandDispatcher &dataDispatcher, EventDispatcher &eventDispatcher) const override;

	void onEvent(const DeathEvent &event);
	void onEvent(const AnimationEndEvent &event);
};