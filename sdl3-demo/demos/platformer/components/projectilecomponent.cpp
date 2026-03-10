#include "projectilecomponent.h"

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner)
{
	hit = false;
	lifeDuration = 0;
	deathTime = 0;
}


