#include "projectilecomponent.h"

#include <node.h>
#include <world.h>
#include <messaging/events.h>
#include <messaging/eventdispatcher.h>
#include <messaging/commands.h>
#include <messaging/eventqueue.h>
#include <resources.h>
#include <components/collisioncomponent.h>

#include "../events.h"

ProjectileComponent::ProjectileComponent(Node &owner) : Component(owner, FrameStage::Gameplay)
{
	hit = false;
	/*services.registerHandler<CollisionEvent>(this);
	owner.getEventDispatcher().registerHandler<NodeRemovalEvent>(this);*/
}


