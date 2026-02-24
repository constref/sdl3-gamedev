#include "empty.h"

#include <node.h>
#include <sdlstate.h>
#include <world.h>
#include <components/animationcomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>
#include <components/basiccameracomponent.h>
#include <components/spritecomponent.h>
#include <systems/systemregistry.h>
#include <componentsystems.h>
#include <prototypeinstancer.h>
#include <messaging/events.h>
#include <resourceloader.h>

Empty::Empty()
{
}

void Empty::start(Services &services, SDLState &state)
{
}

bool Empty::initialize(Services &services, SDLState &state)
{
	World &world = services.world();
	setRoot(world.createNode());
	return true;
}
