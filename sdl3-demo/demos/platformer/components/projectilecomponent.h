#pragma once

#include <components/component.h>
#include <resourceid.h>

class ProjectileComponent : public Component
{
	bool hit;
	float lifeDuration;

public:
	ProjectileComponent(Node &owner);
	bool hasHit() const { return hit; }
	void setHasHit(bool hit) { this->hit = hit; }
	void setLifeDuration(float duration) { this->lifeDuration = duration; }
	float getLifeDuration() const { return lifeDuration; }

	ResourceId animProjectileHitId;
	ResourceId texProjectileHitId;
	float deathTime;
};