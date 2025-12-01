#include "projectilecomponent.h"

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, FrameStage::Gameplay)
{
	hit = false;
	lifeDuration = 0;
}


