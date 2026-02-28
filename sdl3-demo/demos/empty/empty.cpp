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

#include <systems/d3d12/d3d12rendersystem.h>

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

	auto *renderSys = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();

	d3d12rs::Mesh boxMesh;
	boxMesh.addSubmesh(d3d12rs::SubMesh{
		.vertices = {
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1, 0, 0, 1) },
			{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0, 1, 0, 1) },
			{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(0, 0, 1, 1) },
			{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(1, 1, 0, 1) },
			{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 1, 1) },
			{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(0, 1, 1, 1) },
			{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(1, 1, 0, 1) },
			{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 0, 1) }
		},
		.indices = {
			0, 1, 2, 0, 2, 3, // front face
			4, 6, 5, 4, 7, 6, // back face
			4, 5, 1, 4, 1, 0, // left face
			3, 2, 6, 3, 6, 7, // right face
			1, 5, 6, 1, 6, 2, // top face
			4, 0, 3, 4, 3, 7  // bottom face
		}
	});

	GPUMeshHandle boxHandle = renderSys->loadMesh(boxMesh);

	Node &root = world.getNode(getRoot());
	services.compSys().addComponent<MeshComponent>(root, boxHandle);

	return true;
}
