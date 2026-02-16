#pragma once

#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <span>
#include <shaderc/shaderc.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <ext/vk_mem_alloc.h>

#include <systems/system.h>
#include <components/spritecomponent.h>
#include <resourceid.h>
#include <nodehandle.h>
#include <messaging/events.h>
#include <tooling/exportedresources.h>

struct SDL_Window;
struct VmaAllocator_T;
typedef struct VmaAllocator_T *VmaAllocator;
struct VmaAllocation_T;
typedef struct VmaAllocation_T *VmaAllocation;

namespace vks
{

enum class Topology
{
	triangle_list,
	triangle_strip
};

struct PipelineConfig
{
	Topology topology = Topology::triangle_list;
};

struct Buffer
{
	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
};

struct Image
{
	VkImage handle = nullptr;
	VkImageView view = nullptr;
	VmaAllocation allocation = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channels = 0;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec2 uv;
};

struct SubMesh
{
	size_t vertexStart = 0;
	size_t vertexCount = 0;
	size_t indexStart = 0;
	size_t indexCount = 0;
};

struct Mesh
{
	std::vector<SubMesh> subMeshes;
};

struct DrawConstants
{
	uint64_t vertexBufferAddress = 0;
	float globalTime = 0;
	float padding = 0;
	glm::mat4 mvp;
	uint32_t textureIndex = 0;
	uint32_t frameNumber = 0;
	uint32_t frameCount = 1;
	uint32_t width = 0;
	uint32_t height = 0;
	float flipH = 1.0f;
	float layerIndex = 0;
};

struct InstanceData
{
	uint64_t vertexBufferAddress = 0;
	float globalTime = 0;
	float padding = 0;
	glm::mat4 mvp;
	uint32_t textureIndex = 0;
	uint32_t frameNumber = 0;
	uint32_t frameCount = 1;
	uint32_t width = 0;
	uint32_t height = 0;
	float flipH = 1.0f;
	float layerIndex = 0;
};

struct ShaderSet
{
	VkShaderModule vert = nullptr;
	VkShaderModule frag = nullptr;
};

struct Pipeline
{
	VkPipelineLayout layout = nullptr;
	VkPipeline handle = nullptr;
};

struct Material
{
	Pipeline pipeline;
};

struct FrameResources
{
	uint32_t lastFrameId = 0;
	VkCommandPool commandPool = nullptr;
	VkCommandBuffer commandBuffer = nullptr;
	Image renderTarget;
	VkDeviceMemory renderTargetMem = nullptr;
	Buffer indirectDraws;
	VkDrawIndexedIndirectCommand *drawCommands = nullptr;
	uint32_t numDraws = 0;
	Buffer instanceData;
	InstanceData *instances = nullptr;
	uint32_t numInstances = 0;
	VkDescriptorSet descSet = nullptr;
	uint64_t vertBufferAddr = 0;
	VmaPool vmaExportPool = nullptr;
};

struct Barrier
{
	VkImage image = nullptr;
	VkPipelineStageFlags2 srcStageMask = 0;
	VkAccessFlags2 srcAccessMask = 0;
	VkPipelineStageFlags2 dstStageMask = 0;
	VkAccessFlags2 dstAccessMask = 0;
	VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
};

class VulkanRenderSystem : public System<FrameStage::Render, SpriteComponent>
{
	constexpr static uint32_t VulkanVersion{ VK_API_VERSION_1_4 };
	constexpr static uint32_t MaxFramesInFlight{ Config::ExecSelect(2, 3) };
	constexpr static VkFormat swapchainFormat{ VK_FORMAT_B8G8R8A8_SRGB };
	constexpr static VkFormat exportFormat{ VK_FORMAT_R8G8B8A8_UNORM };
	constexpr static VkFormat depthFormat{ VK_FORMAT_D32_SFLOAT };
	constexpr static size_t MaxTextures = 1024;
	constexpr static size_t MaxDrawCommands = 5000;
	constexpr static size_t MaxInstances = 5000;

	bool ownedWindow = true;
	SDL_Window *window = nullptr;

	uint64_t prevTime = 0;
	uint64_t nowTime = 0;
	double globalTime = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t logW = 0;
	uint32_t logH = 0;
	bool running = false;
	uint64_t frameCounter = 0;
	uint64_t timelineValue = MaxFramesInFlight - 1; // subtract 1 to ensure wait-for-ID / frame resource index start at 0 during render, avoids if (frameId < MaxFramesInFlight) check

	// vulkan core
	VkInstance vulkanInstance = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice device = nullptr;
	VkSurfaceKHR surface = nullptr;
	VmaAllocator vmaAllocator = nullptr;

	// queue related
	uint32_t gfxQueueFamIdx = UINT32_MAX;
	VkQueue gfxQueue = nullptr;
	VkCommandPool commandPool = nullptr;

	// swapchain related
	VkSwapchainKHR swapchain = nullptr;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkSemaphore> workCompleteSemaphores;
	std::vector<VkSemaphore> imageReadySemaphores;
	bool requireSwapchainRecreate = false;
	uint32_t swapchainWidth = 0;
	uint32_t swapchainHeight = 0;

	VkImage depthImage = nullptr;
	VkImageView depthImageView = nullptr;
	VmaAllocation depthImageAllocation = nullptr;

	// graphics pipeline related
	vks::Pipeline pipeline;
	vks::Pipeline spritePipeline;

	// shader resources
	size_t regShader, spriteShader;
	std::vector<std::unique_ptr<vks::ShaderSet>> shaders;

	// frame and synchronization resources
	VkSemaphore timelineSemaphore = nullptr;
	std::array<vks::FrameResources, MaxFramesInFlight> frameResources;
	VkDeviceSize internalTextureByteSize = 0;
	uint32_t frameResIndex = 0;
	uint32_t workCompleteSemaphoreIndex = 0;
	uint32_t imageReadySemaphoreIndex = 0;
	uint64_t frameId = 0;
	uint64_t waitForId = 0;
	uint32_t imageIndex = 0;

	// resources
	std::vector<vks::Mesh> meshes;
	std::vector<vks::Vertex> vertices;
	std::vector<uint32_t> indices;
	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;

	VkDescriptorSetLayout globalDSLayout = nullptr;
	VkDescriptorSetLayout frameDSLayout = nullptr;
	VkDescriptorSet globalDescSet = nullptr;
	VkDescriptorPool descPool = nullptr;

	VkSampler nearestSampler = nullptr;
	std::vector<vks::Image> images;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData);
	void showError(const std::string &errorMessasge) const;

	bool initializeVulkan();
	bool createVulkanInstance();
	bool createSurface();
	VkPhysicalDevice findPhysicalDevice();
	bool findGraphicsQueue();
	bool createDevice(VkPhysicalDevice physicalDevice);
	bool initializeVMA();
	bool createSwapchain(uint32_t width, uint32_t height);
	bool createWorkSemaphores();
	void destroySwapchain();
	VkShaderModule createShaderModule(const std::string &fileName, shaderc_shader_kind kind) const;
	vks::ShaderSet *createShaders(const std::string &shaderName);
	vks::Pipeline createGraphicsPipeline(const vks::ShaderSet &shaderSet, const vks::PipelineConfig &config) const;
	bool createSyncResources();
	bool createCommandBuffers();
	bool createDescriptorSets();
	VkSampler createSampler();
	vks::Buffer createBuffer(VkBufferUsageFlags usage, VkBufferCreateFlags flags, size_t byteSize, void *initData);
	bool createInternalTargets();
	bool createIndirectDrawBuffers();
	bool updatePerFrameDescriptors();

	void loadModel();
	VkCommandBuffer startTransientCommandBuffer();
	void submitTransientCommandBuffer(VkCommandBuffer commandBuffer, VkFence waitFence = nullptr);
	std::tuple<ResourceId, vks::Image> createImage(uint32_t width, uint32_t height, uint32_t channels);
	void transitionImages(VkCommandBuffer commandBuffer, const std::span<vks::Barrier> &barriers);

public:
	VulkanRenderSystem(SDL_Window *window, uint32_t width, uint32_t height, uint32_t logW, uint32_t logH, Services &services);
	~VulkanRenderSystem();
	bool initialize();
	void shutdown();
	void beginFrame() override;
	void update(Node &node) override;
	void endFrame() override;

	ResourceId loadTexture(const std::string &filepath, bool flipY = false);
	void updateTextures();

	ExportedResources getSharedRenderTarget(int frameIndex);
	intptr_t exportWorkCompleteSemaphore(uint32_t index) const;
	intptr_t exportImageReadySemaphore(uint32_t index) const;
	RenderInfo getRenderInfo() const;

	void onEvent(NodeHandle target, const AnimationPlayEvent &event);
	void onEvent(NodeHandle target, const AnimationStopEvent &event);
	void onEvent(NodeHandle target, const DirectionChangedEvent &event);
};

}
