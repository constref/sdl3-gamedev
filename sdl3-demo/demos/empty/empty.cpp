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

	//Mesh boxMesh;
	//boxMesh.addSubmesh(SubMesh{
	//	.vertices = {
	//		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1, 0, 0, 1) },
	//		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0, 1, 0, 1) },
	//		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(0, 0, 1, 1) },
	//		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(1, 1, 0, 1) },
	//		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 1, 1) },
	//		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(0, 1, 1, 1) },
	//		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(1, 1, 0, 1) },
	//		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1, 0, 0, 1) }
	//	},
	//	.indices = {
	//		0, 1, 2, 0, 2, 3, // front face
	//		4, 6, 5, 4, 7, 6, // back face
	//		4, 5, 1, 4, 1, 0, // left face
	//		3, 2, 6, 3, 6, 7, // right face
	//		1, 5, 6, 1, 6, 2, // top face
	//		4, 0, 3, 4, 3, 7  // bottom face
	//	}
	//	});

	Node &root = world.getNode(getRoot());

	auto *renderSys = services.compSys().getSystemRegistry().getSystem<d3d12rs::D3D12RenderSystem>();
	USDProcessor usdproc;
	auto newMesh = usdproc.loadStage();

	GPUMeshHandle hMesh = renderSys->loadMesh(newMesh);
	services.compSys().addComponent<MeshComponent>(root, hMesh);

	return true;
}
//
//void loadModel()
//{
//	using namespace tinygltf;
//	using namespace DirectX;
//	Model model;
//
//	tinygltf::LoadImageDataFunction imageLoaderFunc = [](tinygltf::Image *image, const int imgIndex, std::string *err, std::string *warn, int reqWidth, int reqHeight, const unsigned char *bytes, int size, void *userData) -> bool
//	{
//		return false;
//	};
//
//	TinyGLTF loader;
//	loader.SetImageLoader(imageLoaderFunc, nullptr);
//
//	std::string err;
//	std::string warn;
//	//loader.LoadASCIIFromFile(&model, &err, &warn, "C:/Users/nikol/Desktop/untitled.gltf");
//	loader.LoadASCIIFromFile(&model, &err, &warn, "D:/glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
//	//loader.LoadASCIIFromFile(&model, &err, &warn, "S:/projects/boiler-3d/data/sorceress/scene.gltf");
//
//	// load images
//	//VkCommandBuffer commandBuffer = startTransientCommandBuffer();
//	//if (commandBuffer)
//	//{
//	//	for (const tinygltf::Image &image : model.images)
//	//	{
//	//		auto [imageId, img] = createImage(image.width, image.height, image.component);
//	//		if (!imageId.isValid())
//	//		{
//	//			showError("Image could not be loaded");
//	//			break;
//	//		}
//	//	}
//	//}
//	//submitTransientCommandBuffer(commandBuffer);
//
//	// load all meshes first
//	for (const tinygltf::Mesh &mesh : model.meshes)
//	{
//		d3d12rs::Mesh newMesh;
//		for (const Primitive &primitive : mesh.primitives)
//		{
//			d3d12rs::SubMesh subMesh;
//			// load primitive vertices into sub-mesh
//			if (const auto &itr = primitive.attributes.find("POSITION"); itr != primitive.attributes.end())
//			{
//				const auto &[name, index] = *itr;
//				const Accessor &access = model.accessors[index];
//				const BufferView &bv = model.bufferViews[access.bufferView];
//				const tinygltf::Buffer &buffer = model.buffers[bv.buffer];
//
//				if (access.type == TINYGLTF_TYPE_VEC3)
//				{
//					for (int i = 0; i < access.count; ++i)
//					{
//						size_t offset = bv.byteOffset + access.byteOffset + i * ((bv.byteStride > 0) ? bv.byteStride : sizeof(glm::vec3));
//						const float *pos = reinterpret_cast<const float *>(buffer.data.data() + offset);
//
//						d3d12rs::Vertex v;
//						XMStoreFloat3(&v.position, XMVECTOR(pos[0], pos[1], pos[2]));
//						XMStoreFloat4(&v.color, XMVECTOR(1, 1, 1, 1));
//						subMesh.vertices.push_back(v);
//					}
//				}
//			}
//			// load primitive vertices into sub-mesh
//			//if (const auto &itr = primitive.attributes.find("TEXCOORD_0"); itr != primitive.attributes.end())
//			//{
//			//	const auto &[name, index] = *itr;
//			//	const Accessor &access = model.accessors[index];
//			//	const BufferView &bv = model.bufferViews[access.bufferView];
//			//	const tinygltf::Buffer &buffer = model.buffers[bv.buffer];
//
//			//	if (access.type == TINYGLTF_TYPE_VEC2)
//			//	{
//			//		for (int i = 0; i < access.count; ++i)
//			//		{
//			//			size_t offset = bv.byteOffset + access.byteOffset + i * ((bv.byteStride > 0) ? bv.byteStride : sizeof(glm::vec2));
//			//			const glm::vec2 *uv = reinterpret_cast<const glm::vec2 *>(buffer.data.data() + offset);
//			//			vertices[subMesh.vertexStart + i].uv = *uv;
//			//		}
//			//	}
//			//}
//			// indices
//			if (primitive.indices != -1)
//			{
//				const Accessor &access = model.accessors[primitive.indices];
//				const BufferView &bv = model.bufferViews[access.bufferView];
//				const tinygltf::Buffer &buffer = model.buffers[bv.buffer];
//
//				assert()
//				if (access.type == TINYGLTF_TYPE_SCALAR && access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
//				{
//					for (int i = 0; i < access.count; ++i)
//					{
//						const uint32_t *idx = reinterpret_cast<const uint32_t *>(buffer.data.data() + bv.byteOffset + access.byteOffset) + i;
//						subMesh.indices.push_back(*idx);
//					}
//				}
//				else if (access.type == TINYGLTF_TYPE_SCALAR && access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
//				{
//					for (int i = 0; i < access.count; ++i)
//					{
//						const uint16_t *idx = reinterpret_cast<const uint16_t *>(buffer.data.data() + bv.byteOffset + access.byteOffset) + i;
//						indices.push_back(*idx);
//					}
//				}
//			}
//			newMesh.subMeshes.push_back(subMesh);
//		}
//		meshes.push_back(newMesh);
//	}
//
//	auto createBuffer = [](VmaAllocator &vmaAllocator, VkBufferUsageFlags usage, size_t byteSize, void *initData)
//	{
//		VkBufferCreateInfo buffInfo
//		{
//			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//			.size = byteSize,
//			.usage = usage,
//			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
//		};
//
//		VmaAllocationCreateInfo allocInfo
//		{
//			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
//			.usage = VMA_MEMORY_USAGE_CPU_TO_GPU
//		};
//
//		vks::Buffer newBuff;
//		if (vmaCreateBuffer(vmaAllocator, &buffInfo, &allocInfo, &newBuff.buffer, &newBuff.allocation, nullptr) != VK_SUCCESS)
//		{
//			//showError("Error allocating buffer");
//		}
//
//		void *buffPtr = nullptr;
//		if (vmaMapMemory(vmaAllocator, newBuff.allocation, &buffPtr) != VK_SUCCESS)
//		{
//			//showError("Unable to map buffer memory");
//		}
//		std::memcpy(static_cast<char *>(buffPtr), initData, buffInfo.size);
//		const glm::vec3 *vec3Ptr = reinterpret_cast<const glm::vec3 *>(buffPtr);
//		vmaUnmapMemory(vmaAllocator, newBuff.allocation);
//
//		return newBuff;
//	};
//
//	vertexBuffer = createBuffer(vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
//		sizeof(Vertex) * vertices.size(), vertices.data());
//	indexBuffer = createBuffer(vmaAllocator, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
//		sizeof(uint32_t) * indices.size(), indices.data());
//}

