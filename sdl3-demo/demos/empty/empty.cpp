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
#include <fstream>
#include <prototypeinstancer.h>
#include <messaging/events.h>
#include <resourceloader.h>
#include <ext/uuid.h>
#include <persistence/project.h>
#include <vulkan/vulkan_core.h>

#include "inputstate.h"
#include "rendering/mesh.h"
#include "rendering/renderer.h"
#include "rendering/vertex.h"
#include "systems/fpscamerasystem.h"
#include "systems/d3d12/d3d12rendersystem.h"
#include "tooling/systems/editorsystem.h"
#include "tooling/systems/editorinputsystem.h"
#include "tooling/systems/nodeauthoringcomponent.h"
#include "components/lightingcomponent.h"


#include <assets/assetmanager.h>
#include <assets/meshloader.h>

using namespace DirectX;

bool Empty::initialize(Services& services, SDLState& state)
{
    if constexpr (Config::IsToolingMode())
    {
        // TODO: Toggle system registration for tooling builds appropriately
        services.compSys().registerSystem(std::make_unique<EditorInputSystem>(services));
        services.compSys().registerSystem(std::make_unique<EditorSystem>(services));
    }
    services.compSys().registerSystem(std::make_unique<FPSCameraSystem>(services));

    return true;
}

void Empty::start(Services& services, SDLState& state)
{
    World& world = services.world();
    setRoot(world.createNode());
    Node& root = world.getNode(getRoot());

    NodeHandle hPlayer = world.createNode();
    Node& player = world.getNode(hPlayer);
    player.setPosition(glm::vec3(0, 0, -3));
    auto& physics = services.compSys().addComponent<PhysicsComponent>(player);
    physics.setAcceleration(glm::vec3(30, 30, 30));
    physics.setMaxSpeed(glm::vec3(50, 50, 50));
    physics.setGravityFactor(0);
    services.compSys().addComponent<CollisionComponent>(player);
    auto& input = services.compSys().addComponent<InputComponent>(player);
    input.setAxes(0, 2, 1); // A/D controls X-axis, W/S controls Z-axis
    services.inputState().setFocus(hPlayer);
    services.compSys().addComponent<CameraComponent>(player);
    root.addChild(player);

    /*
    std::ifstream file("demo.nub", std::ios::binary);
    uint32_t nodeCount = 0;
    file.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));

    // read all nodes
    std::vector<persistence::Node> nodes(nodeCount);
    file.read(reinterpret_cast<char*>(nodes.data()), nodeCount * sizeof(persistence::Node));

    // read mesh data
    std::vector<Mesh> meshes;
    uint32_t meshCount = 0;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
    for (int i = 0; i < meshCount; ++i)
    {
        persistence::Mesh meshHeader;
        file.read(reinterpret_cast<char*>(&meshHeader), sizeof(persistence::Mesh));

        // build sub-mesh objects
        Mesh mesh;
        for (uint32_t j = 0; j < meshHeader.subMeshCount; ++j)
        {
            persistence::SubMesh submeshHeader;
            file.read(reinterpret_cast<char*>(&submeshHeader), sizeof(persistence::SubMesh));

            std::vector<Vertex> vertices(submeshHeader.vertexCount);
            file.read(reinterpret_cast<char*>(vertices.data()), submeshHeader.vertexCount * sizeof(Vertex));
            std::vector<uint16_t> indices(submeshHeader.indexCount);
            file.read(reinterpret_cast<char*>(indices.data()), submeshHeader.indexCount * sizeof(uint16_t));
            mesh.addSubmesh(SubMesh(vertices, indices));
        }
        meshes.push_back(std::move(mesh));
    }
    file.close();

    d3d12rs::D3D12RenderSystem* renderer = services.compSys().getSystemRegistry().getSystem<
        d3d12rs::D3D12RenderSystem>();

    // load meshes into GPU
    std::vector<GPUMeshHandle> gpuMeshHandles;
    for (const Mesh &mesh : meshes)
    {
        gpuMeshHandles.push_back(renderer->loadMesh(mesh));
    }
    renderer->executeAssetCopyOps(); // TODO: This shouldn't be here
    
    for (const auto& n : nodes)
    {
        NodeHandle hNode = services.world().createNode();
        Node& node = services.world().getNode(hNode);
        node.setPosition(glm::vec3(n.position.x, n.position.y, n.position.z));
        node.setRotation(n.rotation);

        uuids::uuid nodeId;
        if (!n.meshId.isNull())
        {
            auto itr = std::find(meshes.begin(), meshes.end(), n.meshId);
            assert(itr != meshes.end() && "Mesh ID not found");
            
            size_t index = std::distance(meshes.begin(), itr);
            GPUMeshHandle gpuMeshHandle = gpuMeshHandles[index];
            services.compSys().addComponent<MeshComponent>(node, gpuMeshHandle);
        }
        else
        {
            exit(1);
        }
		root.addChild(node);
    }
    */

    LoadResult result = assets::loadGltf("S:/projects/constref/sdl3-demo/data/new_triptych/sphere.gltf");
    auto assetUuid = uuids::uuid::from_string("917f5503-01d2-411b-b9f7-ed5b54ca7697");
    services.assetManager().loadMesh(assetUuid.value(), std::move(result.meshes.at("Sphere")));

    NodeHandle hNode = services.world().createNode();
    Node &node = services.world().getNode(hNode);
    auto &meshComp = services.compSys().addComponent<MeshComponent>(node, assetUuid.value());
    root.addChild(node);

    NodeHandle hLight = services.world().createNode();
    Node &light = services.world().getNode(hLight);
    light.setPosition(glm::vec3(-2, 3, 0));
    services.compSys().addComponent<LightingComponent>(light);
    root.addChild(light);
}
