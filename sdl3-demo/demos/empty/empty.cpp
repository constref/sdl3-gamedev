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
#include "systems/fpscamerasystem.h"
#include "tooling/systems/editorsystem.h"
#include "tooling/systems/editorinputsystem.h"
#include "tooling/systems/nodeauthoringcomponent.h"

using namespace DirectX;

bool Empty::initialize(Services &services, SDLState &state)
{
	// TODO: Toggle system registration for tooling builds appropriately
	services.compSys().registerSystem(std::make_unique<EditorInputSystem>(services));
	services.compSys().registerSystem(std::make_unique<EditorSystem>(services));
	
	services.compSys().registerSystem(std::make_unique<FPSCameraSystem>(services));
	return true;
}

void Empty::start(Services &services, SDLState &state)
{
	World &world = services.world();
	setRoot(world.createNode());
	Node &root = world.getNode(getRoot());
	
	NodeHandle hPlayer = world.createNode();
	Node &player = world.getNode(hPlayer);
	player.setPosition(glm::vec3(5, 0.6f, -1));
	auto &physics = services.compSys().addComponent<PhysicsComponent>(player);
	physics.setAcceleration(glm::vec3(30, 30, 30));
	physics.setMaxSpeed(glm::vec3(50, 50, 50));
	physics.setGravityFactor(0);
	services.compSys().addComponent<CollisionComponent>(player);
	auto &input = services.compSys().addComponent<InputComponent>(player);
	input.setAxes(0, 2, 1); // A/D controls X-axis, W/S controls Z-axis
	services.inputState().setFocus(hPlayer);
	services.compSys().addComponent<CameraComponent>(player);
	
	root.addChild(player);

	// usd::UsdProcessor proc;
	// proc.openStage("S:\\projects\\constref\\sdl3-demo\\level_LAYOUT.usda");
	// proc.bakeStage(root, services);
}
