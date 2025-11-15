#pragma once

#include <components/component.h>

class ProjectileComponent : public Component
{
	bool hit;

public:
	ProjectileComponent(Node &owner);
	bool hasHit() const { return hit; }
	void setHasHit(bool hit) { this->hit = hit; }
};