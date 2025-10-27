#include "projectilecomponent.h"

ProjectileComponent::ProjectileComponent(GameObject &owner) : Component(owner, ComponentStage::Gameplay)
{
}

void ProjectileComponent::update(const FrameContext &ctx)
{
}

void ProjectileComponent::onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
}
