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

#include "inputstate.h"
#include "tooling/usd/usdprocessor.h"

using namespace DirectX;

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
	Node &root = world.getNode(getRoot());
	
	NodeHandle hPlayer = world.createNode();
	Node &player = world.getNode(hPlayer);
	services.compSys().addComponent<PhysicsComponent>(player);
	services.compSys().addComponent<CollisionComponent>(player);
	services.compSys().addComponent<InputComponent>(player);
	services.inputState().setFocus(hPlayer);
	
	//const std::string usdPath = "data\\usd\\ufo.usd";
	const std::string usdPath = "C:/Users/nikol/Documents/maya/projects/USD Concept/usd/prefabs_MODEL.usd";

	USDProcessor usdproc;
	usdproc.loadStage(usdPath, root, services);

	return true;
}
