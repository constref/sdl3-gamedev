#include "vulkanrendersystem.h"
#include <systems/context/rendercontext.h>
#include <components/spritecomponent.h>

#include <SDL3/SDL.h>
#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#include <tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>

constexpr bool debugging = true;

std::string readTextFile(const std::string &filePath)
{
	std::ifstream infile(filePath);
	if (infile.is_open())
	{
		std::stringstream buffer;
		buffer << infile.rdbuf();
		const std::string output = buffer.str();
		infile.close();
		return output;
	}
	return std::string();
}

using namespace vks;

void VulkanRenderSystem::showError(const std::string &errorMessage) const
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), window);
}

VulkanRenderSystem::VulkanRenderSystem(SDL_Window *window, uint32_t width, uint32_t height, uint32_t logW, uint32_t logH, Services &services) : System(services), window(window)
{
	this->width = width;
	this->height = height;
	this->logW = logW;
	this->logH = logH;
	if (window)
	{
		ownedWindow = false;
	}

	services.eventQueue().dispatcher.registerHandler<AnimationPlayEvent>(this);
	services.eventQueue().dispatcher.registerHandler<AnimationStopEvent>(this);
	services.eventQueue().dispatcher.registerHandler<DirectionChangedEvent>(this);
}

VulkanRenderSystem::~VulkanRenderSystem()
{
	shutdown();
}

bool VulkanRenderSystem::initialize()
{
	if (!initializeVulkan())
	{
		return false;
	}

	std::vector<Vertex> quad;
	quad.reserve(4);
	quad.push_back(Vertex{ .position = { -0.5, -0.5, 0.0 }, .uv = {0, 1} });
	quad.push_back(Vertex{ .position = { -0.5, 0.5, 0.0 }, .uv = {0, 0} });
	quad.push_back(Vertex{ .position = { 0.5,  -0.5, 0.0 }, .uv = {1, 1} });
	quad.push_back(Vertex{ .position = { 0.5,  0.5, 0.0 }, .uv = {1, 0} });

	for (int i = 0; i < quad.size(); ++i)
	{
		vertices.push_back(quad[i]);
		indices.push_back(i);
	}
	vertexBuffer = createBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		0, sizeof(Vertex) * vertices.size(), vertices.data());
	indexBuffer = createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, sizeof(uint32_t) * indices.size(), indices.data());

	Mesh newMesh;
	SubMesh sm;
	sm.vertexStart = 0;
	sm.vertexCount = 4;
	sm.indexCount = 4;
	sm.indexStart = 0;
	newMesh.subMeshes.push_back(sm);
	meshes.push_back(newMesh);

	//loadModel();

	return true;
}

void VulkanRenderSystem::shutdown()
{
	// wait in case resources are in use
	vkDeviceWaitIdle(device);

	if (nearestSampler)
	{
		vkDestroySampler(device, nearestSampler, nullptr);
	}

	if (globalDSLayout)
	{
		vkDestroyDescriptorSetLayout(device, globalDSLayout, nullptr);
	}
	if (frameDSLayout)
	{
		vkDestroyDescriptorSetLayout(device, frameDSLayout, nullptr);
	}
	if (descPool)
	{
		vkDestroyDescriptorPool(device, descPool, nullptr);
	}

	// clean up images
	for (const Image &image : images)
	{
		vmaDestroyImage(vmaAllocator, image.handle, image.allocation);
		vkDestroyImageView(device, image.view, nullptr);
	}
	images.clear();

	// single-use command buffer pool
	vkDestroyCommandPool(device, commandPool, nullptr);

	// destroy allocated buffers
	vmaDestroyBuffer(vmaAllocator, vertexBuffer.buffer, vertexBuffer.allocation);
	vmaDestroyBuffer(vmaAllocator, indexBuffer.buffer, indexBuffer.allocation);

	// frame / sync object cleanup
	if (timelineSemaphore)
	{
		vkDestroySemaphore(device, timelineSemaphore, nullptr);
	}
	for (auto &res : frameResources)
	{
		vkDestroySemaphore(device, res.imageAcquiredSemaphore, nullptr);
		vkDestroySemaphore(device, res.workCompleteSemaphore, nullptr);
		vkDestroyCommandPool(device, res.commandPool, nullptr); // destroys buffers implicitly

		// cleanup internal render targets
		if (res.renderTarget.view)
		{
			vkDestroyImageView(device, res.renderTarget.view, nullptr);
		}
		if (res.renderTarget.handle)
		{
			vmaDestroyImage(vmaAllocator, res.renderTarget.handle, res.renderTarget.allocation);
		}

		// draw and instance data cleanup
		if (res.indirectDraws.buffer)
		{
			vmaUnmapMemory(vmaAllocator, res.indirectDraws.allocation);
			vmaDestroyBuffer(vmaAllocator, res.indirectDraws.buffer, res.indirectDraws.allocation);
		}
		if (res.instanceData.buffer)
		{
			vmaUnmapMemory(vmaAllocator, res.instanceData.allocation);
			vmaDestroyBuffer(vmaAllocator, res.instanceData.buffer, res.instanceData.allocation);
		}
	}

	// destroy the depth buffer along with the swapchain
	if (depthImageView)
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		vmaDestroyImage(vmaAllocator, depthImage, depthImageAllocation);
		depthImageView = nullptr;
	}

	// pipeline cleanup
	if (pipeline.layout)
	{
		vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
	}
	if (pipeline.handle)
	{
		vkDestroyPipeline(device, pipeline.handle, nullptr);
	}
	if (spritePipeline.layout)
	{
		vkDestroyPipelineLayout(device, spritePipeline.layout, nullptr);
	}
	if (spritePipeline.handle)
	{
		vkDestroyPipeline(device, spritePipeline.handle, nullptr);
	}

	// cleanup shaders
	for (auto &set : shaders)
	{
		if (set->vert)
		{
			vkDestroyShaderModule(device, set->vert, nullptr);
		}
		if (set->frag)
		{
			vkDestroyShaderModule(device, set->frag, nullptr);
		}
	}
	shaders.clear();

	// cleanup swapchain
	destroySwapchain();

	// VMA
	if (vmaAllocator)
	{
		vmaDestroyAllocator(vmaAllocator);
	}

	// cleanup Vulkan
	if (surface)
	{
		vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
	}
	if (device)
	{
		vkDestroyDevice(device, nullptr);
	}
	if (vulkanInstance)
	{
		vkDestroyInstance(vulkanInstance, nullptr);
	}
	volkFinalize();

	// cleanup SDL if we own it
	if (ownedWindow)
	{
		if (window)
		{
			SDL_DestroyWindow(window);
		}
		SDL_Quit();
	}
}

void VulkanRenderSystem::beginFrame()
{
	// first check if our swapchain is still valid
	if (requireSwapchainRecreate)
	{
		vkDeviceWaitIdle(device);
		destroySwapchain();
		if (!createSwapchain(width, height))
		{
			showError("Unable to recreate the swapchain");
			running = false;
			return;
		}
		requireSwapchainRecreate = false;
	}

	frameResIndex = frameCounter++ % MaxFramesInFlight;
	// wait for frame using this frame's resources to complete
	frameId = ++timelineValue; // this is our frame "ID", and what we're using to signal the end of this frame later
	waitForId = frameId - MaxFramesInFlight; // frame N and frame N - MaxInFlight share resources (3 - 2 = 1 -- frame 3 and 1 share resources)

	VkSemaphoreWaitInfo waitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = &timelineSemaphore,
		.pValues = &waitForId
	};
	vkWaitSemaphores(device, &waitInfo, UINT64_MAX);

	// now its safe to start recording commands
	FrameResources &res = frameResources[frameResIndex];
	res.numDraws = 0;
	res.numInstances = 0;

	// BDA Send Device Pointer
	VkBufferDeviceAddressInfo vertBdaInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = vertexBuffer.buffer };
	res.vertBufferAddr = vkGetBufferDeviceAddress(device, &vertBdaInfo);

	vkResetCommandPool(device, res.commandPool, 0); // resets all buffers

	// begin recording commands
	VkCommandBufferBeginInfo cmdBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(res.commandBuffer, &cmdBeginInfo);

	// transition the color and depth images
	std::array<Barrier, 2> layoutBarriers
	{
		Barrier {
			.image = res.renderTarget.handle,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		},
		{
			.image = depthImage,
			.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, // both specified to control memory access at both stages (write)
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, // both specified to control memory access at both stages (write)
			.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT
		}
	};
	transitionImages(res.commandBuffer, layoutBarriers);

	std::array<VkDescriptorSet, 2> sets{ globalDescSet, res.descSet };
	vkCmdBindDescriptorSets(res.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.layout, 0, sets.size(), sets.data(), 0, nullptr);
}

void VulkanRenderSystem::endFrame()
{
	FrameResources &res = frameResources[frameResIndex];

	// ensure indirect draw and instance data has been flushed to VRAM
	VkMemoryBarrier barrier
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT
	};
	vkCmdPipelineBarrier(res.commandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 1, &barrier, 0, nullptr, 0, nullptr
	);

	// setup the attachments (color and depth) and begin rendering (dynamic rendering)
	VkRenderingAttachmentInfo colorAttachInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = res.renderTarget.view,
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the image
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // keep data for presentation
		.clearValue{.color{0.0f, 0.0f, 0.0f, 1}}
	};
	VkRenderingAttachmentInfo depthAttachInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = depthImageView,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the depth data
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // don't care after rendering
		.clearValue{.depthStencil{1.0f, 0}}
	};
	VkRenderingInfo renderingInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea
		{
			.offset{.x = 0, .y = 0},
			.extent{.width = logW, .height = logH}
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachInfo,
		.pDepthAttachment = &depthAttachInfo
	};
	// begin dynamic rendering
	vkCmdBeginRendering(res.commandBuffer, &renderingInfo);

	// set the viewpot and scissor state
	VkViewport viewport
	{
		.x = 0, .y = static_cast<float>(logH),
		.width = static_cast<float>(logW),
		.height = -static_cast<float>(logH),
		.minDepth = 0,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(res.commandBuffer, 0, 1, &viewport);

	VkRect2D scissor
	{
		.offset{.x = 0, .y = 0 },
		.extent{.width = logW, .height = logH}
	};
	vkCmdSetScissor(res.commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(res.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.handle);

	vkCmdBindIndexBuffer(res.commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexedIndirect(res.commandBuffer, res.indirectDraws.buffer, 0, res.numDraws, sizeof(VkDrawIndexedIndirectCommand));

	// end internal texture pass
	vkCmdEndRendering(res.commandBuffer);

	// start the swapchain render pass
	// acquire the swapchain image, no need to wait for timeline semaphore just to then wait for the swapchain image
	VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, res.imageAcquiredSemaphore, VK_NULL_HANDLE, &imageIndex);
	// handle resize and out-of-date images, may need swapchain recreate

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		requireSwapchainRecreate = true;
		return;
	}
	else if (acquireResult == VK_SUBOPTIMAL_KHR)
	{
		// can render this frame, recreate next time around
		requireSwapchainRecreate = true;
	}

	// transition the internal render target for blitting onto the swapchain
	std::array<Barrier, 2> blitToSwapTransitions
	{
		Barrier {
			.image = res.renderTarget.handle,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		},
		{
			.image = swapchainImages[imageIndex],
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		}
	};
	transitionImages(res.commandBuffer, blitToSwapTransitions);

	const bool isIntegerScale = false;
	const float scale = isIntegerScale ? std::floor(std::min(swapchainWidth / static_cast<float>(logW), swapchainHeight / static_cast<float>(logH)))
		: std::min(swapchainWidth / static_cast<float>(logW), swapchainHeight / static_cast<float>(logH));
	const int32_t xScaled = logW * scale;
	const int32_t yScaled = logH * scale;
	const int32_t xOffset = (swapchainWidth - xScaled) / 2;
	const int32_t yOffset = (swapchainHeight - yScaled) / 2;

	// blit the internal texture onto the swapchain
	VkClearColorValue clearColor{ 0, 0, 0, 1 };
	VkImageSubresourceRange range
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
	vkCmdClearColorImage(res.commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

	VkImageBlit2 blitRegion
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.srcSubresource {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .layerCount = 1},
		.srcOffsets = {{.x = 0, .y = 0, .z = 0}, {.x = static_cast<int32_t>(logW), .y = static_cast<int32_t>(logH), .z = 1} },
		.dstSubresource {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .layerCount = 1},
		.dstOffsets = {{.x = xOffset, .y = yOffset, .z = 0}, {.x = xOffset + xScaled, .y = yOffset + yScaled, .z = 1}}
	};
	VkBlitImageInfo2 blitInfo
	{
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.srcImage = res.renderTarget.handle,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = swapchainImages[imageIndex],
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &blitRegion,
		.filter = VK_FILTER_NEAREST,
	};

	vkCmdBlitImage2(res.commandBuffer, &blitInfo);

	// transition the swapchain image from blit DST to presentation
	std::array<Barrier, 1> barriers
	{
		Barrier {
			.image = swapchainImages[imageIndex],
			.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
			.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		}
	};
	transitionImages(res.commandBuffer, barriers);

	vkEndCommandBuffer(res.commandBuffer);

	// signal that the image can be presented
	std::vector<VkSemaphoreSubmitInfo> semaphoreSignals
	{
		{ // render work completion signal
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = res.workCompleteSemaphore,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
		},
		{ // entire frame is completed (timeline)
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = timelineSemaphore,
			.value = frameId, // we're signalling our current frame ID is complete
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
		}
	};
	VkCommandBufferSubmitInfo cmdSubmitInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = res.commandBuffer,
	};

	// ensure swapchain image is actually available to start color output
	VkSemaphoreSubmitInfo imageAcquireWaitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = res.imageAcquiredSemaphore,
		.stageMask = VK_PIPELINE_STAGE_2_BLIT_BIT | VK_PIPELINE_STAGE_2_CLEAR_BIT
	};

	VkSubmitInfo2 submitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &imageAcquireWaitInfo, // ensure the image is ready
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
		.signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSignals.size()),
		.pSignalSemaphoreInfos = semaphoreSignals.data()
	};
	vkQueueSubmit2(gfxQueue, 1, &submitInfo, VK_NULL_HANDLE);

	// present the image
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &res.workCompleteSemaphore, // render work completed semaphore
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
		.pResults = nullptr
	};

	vkQueuePresentKHR(gfxQueue, &presentInfo);
}

void VulkanRenderSystem::update(Node &node)
{
	auto [sc] = getRequiredComponents(node);
	FrameResources &res = frameResources[frameResIndex];

	constexpr float fov = glm::radians(45.0);
	float aspect = static_cast<float>(logW) / static_cast<float>(logH);
	float nearP = 0.1f;
	float farP = 32.0f;

	glm::vec3 nodePos = glm::floor(node.getPosition() + glm::vec3(sc->getSize().x / 2.0f, sc->getSize().y / 2.0f, 0));
	glm::vec3 camPos(RenderContext::shared().getCameraPosition().x * sc->getFollowViewport(), RenderContext::shared().getCameraPosition().y * sc->getFollowViewport(), 0);
	glm::vec3 camNodePos = nodePos - camPos;
	if (camNodePos.x > logW + sc->getSize().x || camNodePos.x < -sc->getSize().x) return;

	//glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, nearP, farP);
	glm::mat4 proj = glm::ortho(float(0), float(logW), float(logH), 0.0f, 0.0f, 100.0f);
	glm::mat4 rotation = glm::rotate(glm::mat4(1), static_cast<float>(globalTime), glm::vec3(0, 1, 0));
	glm::mat4 translate = glm::translate(glm::mat4(1), nodePos);
	glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1));
	glm::mat4 transform = translate * rotation * scale;
	const glm::mat4 view = glm::translate(glm::mat4(1), glm::floor(-camPos));
	glm::mat4 mvp = proj * view * transform;

	//DrawConstants pushConsts
	//{
	//	.vertexBufferAddress = vkGetBufferDeviceAddress(device, &vertBdaInfo),
	//	.globalTime = static_cast<float>(globalTime),
	//	.mvp = mvp,
	//	.textureIndex = sc->getTexture().index(),
	//	.frameNumber = static_cast<uint32_t>(sc->getFrameNumber()),
	//	.frameCount = static_cast<uint32_t>(sc->getFrameCount()),
	//	.width = static_cast<uint32_t>(sc->getSize().x),
	//	.height = static_cast<uint32_t>(sc->getSize().y),
	//	.flipH = 1.0f - static_cast<uint32_t>(sc->getFlipH()) * 2.0f,
	//	.layerIndex = static_cast<float>(sc->getLayerIndex())
	//};
	//vkCmdPushConstants(res.commandBuffer, spritePipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DrawConstants), &pushConsts);

	const size_t instanceIdx = res.numInstances++;
	res.instances[instanceIdx] = {
		.vertexBufferAddress = res.vertBufferAddr,
		.globalTime = static_cast<float>(globalTime),
		.mvp = mvp,
		.textureIndex = sc->getTexture().index(),
		.frameNumber = static_cast<uint32_t>(sc->getFrameNumber()),
		.frameCount = static_cast<uint32_t>(sc->getFrameCount()),
		.width = static_cast<uint32_t>(sc->getSize().x),
		.height = static_cast<uint32_t>(sc->getSize().y),
		.flipH = 1.0f - static_cast<uint32_t>(sc->getFlipH()) * 2.0f,
		.layerIndex = static_cast<float>(sc->getLayerIndex())
	};

	for (Mesh &mesh : meshes)
	{
		assert(mesh.subMeshes.size() == 1);
		for (SubMesh &sub : mesh.subMeshes)
		{
			VkDrawIndexedIndirectCommand &cmd = res.drawCommands[res.numDraws++];
			cmd.firstIndex = sub.indexStart;
			cmd.indexCount = sub.indexCount;
			cmd.firstInstance = instanceIdx;
			cmd.instanceCount = 1;
			cmd.vertexOffset = sub.vertexStart;
		}
	}
}

bool VulkanRenderSystem::initializeVulkan()
{
	if (!createVulkanInstance())
	{
		showError("Couldn't create a vulkan instance");
		return false;
	}

	if (!createSurface())
	{
		showError("Couldn't create window surface");
		return false;
	}

	if (physicalDevice = findPhysicalDevice(); !physicalDevice)
	{
		showError("Unable to find an appropriate physical device");
		return false;
	}

	if (!findGraphicsQueue())
	{
		showError("Unable to find a compatible graphics queue");
		return false;
	}

	if (!createDevice(physicalDevice))
	{
		showError("Couldn't create the logical GPU device");
		return false;
	}

	if (!initializeVMA())
	{
		showError("Unable to create Vulkan Memory Allocator");
		return false;
	}

	if (!createSwapchain(width, height))
	{
		showError("Unable to create swapchain");
		return false;
	}

	createInternalTargets();

	ShaderSet *shaderRegular = createShaders("shader");
	if (!shaderRegular)
	{
		showError("Error creating shader modules");
		return false;
	}
	ShaderSet *shaderSprite = createShaders("sprite");
	if (!shaderSprite)
	{
		showError("Error creating shader modules");
		return false;
	}

	if (!createDescriptorSets())
	{
		showError("Error creating descriptor sets");
		return false;
	}

	PipelineConfig pc1{};
	if (pipeline = createGraphicsPipeline(*shaderRegular, pc1); !pipeline.handle)
	{
		showError("Unable to initialize the graphics pipeline");
		return false;
	}
	PipelineConfig pc2{ .topology = Topology::triangle_strip };
	if (spritePipeline = createGraphicsPipeline(*shaderSprite, pc2); !spritePipeline.handle)
	{
		showError("Unable to initialize the graphics pipeline");
		return false;
	}

	if (!createSyncResources())
	{
		showError("Couldn't create the sync related resources");
		return false;
	}

	if (!createCommandBuffers())
	{
		showError("Couldn't create command buffer objects");
		return false;
	}

	if (nearestSampler = createSampler(); !nearestSampler)
	{
		showError("Unable to create nearest sampler");
		return false;
	}

	if (!createIndirectDrawBuffers())
	{
		showError("Unable to create indirect/instanced drawing buffers");
		return false;
	}
	if (!updatePerFrameDescriptors())
	{
		showError("Unable to update the per-frame descriptor sets");
		return false;
	}

	return true;
}

bool VulkanRenderSystem::createVulkanInstance()
{
	// Initialize Volk and load Vk function pointers
	if (volkInitialize() != VK_SUCCESS)
	{
		showError("Error initializing Volk");
		return false;
	}

	// Create the vulkan application instance
	VkApplicationInfo appInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "My First Triangle",
		.apiVersion = VulkanVersion,
	};

	// find the required extensions for the platform and add debug for ourselves
	uint32_t instExtCount = 0;
	const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);
	std::vector<const char *> requestedExtensions
	{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};
	for (int i = 0; i < instExtCount; ++i)
	{
		requestedExtensions.push_back(extensions[i]);
	}


	// we'll also need to enable the validation layer for error checking and reporting
	std::vector<const char *> requestedLayers
	{
		"VK_LAYER_KHRONOS_validation"
	};

	VkDebugUtilsMessengerCreateInfoEXT debugInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback
	};

	VkInstanceCreateInfo instCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &debugInfo,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
		.ppEnabledLayerNames = requestedLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size()),
		.ppEnabledExtensionNames = requestedExtensions.data()
	};

	if (vkCreateInstance(&instCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS)
	{
		return false;
	}

	volkLoadInstance(vulkanInstance);
	return true;
}

bool VulkanRenderSystem::createSurface()
{
	if (!SDL_Vulkan_CreateSurface(window, vulkanInstance, nullptr, &surface))
	{
		return false;
	}

	return true;
}

VkPhysicalDevice VulkanRenderSystem::findPhysicalDevice()
{
	// enumerate all physical devices
	uint32_t physDeviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanInstance, &physDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physDeviceCount);
	vkEnumeratePhysicalDevices(vulkanInstance, &physDeviceCount, physicalDevices.data());

	VkPhysicalDevice physicalDevice = nullptr;
	if (physDeviceCount)
	{
		// if you have issues, you can always just hardcode a GPU index while learning
		physicalDevice = physicalDevices[0]; // default to first GPU
		// look through list and see if a dGPU exists
		for (auto &pDev : physicalDevices)
		{
			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(pDev, &props);
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				physicalDevice = pDev;
				break;
			}
		}
	}

	// ensure the desired swapchain format is supported
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

	bool formatSupported = false;
	for (const VkSurfaceFormatKHR &surfFormat : surfaceFormats)
	{
		if (surfFormat.format == swapchainFormat)
		{
			formatSupported = true;
			break;
		}
	}
	if (!formatSupported)
	{
		showError("Requested swapchain format is not supported by the surface");
		return nullptr;
	}

	return physicalDevice;
}

bool VulkanRenderSystem::findGraphicsQueue()
{
	// eventually we'll have more complex queue lookup for presentation, etc
	// grab all of the queue families
	uint32_t queueFamCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamCount, nullptr);
	std::vector<VkQueueFamilyProperties2> queueFamProps(queueFamCount, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamCount, queueFamProps.data());

	for (int currentFamIdx = 0; currentFamIdx < queueFamProps.size(); currentFamIdx++)
	{
		// ensure it has presentation support
		VkBool32 hasPresentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, currentFamIdx, surface, &hasPresentSupport);

		const auto &props = queueFamProps[currentFamIdx];
		// ensure this is a GRAPHICS queue with presentation support
		if (props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport)
		{
			gfxQueueFamIdx = currentFamIdx;
			return true;
		}
	}
	return false;
}


bool VulkanRenderSystem::createDevice(VkPhysicalDevice physicalDevice)
{
	// query supported features
	VkPhysicalDeviceVulkan14Features supportedFeatures14{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr };
	VkPhysicalDeviceVulkan13Features supportedFeatures13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &supportedFeatures14 };
	VkPhysicalDeviceVulkan12Features supportedFeatures12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &supportedFeatures13 };
	VkPhysicalDeviceFeatures2 supportedFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &supportedFeatures12 };
	vkGetPhysicalDeviceFeatures2(physicalDevice, &supportedFeatures);

	// check if what we need is supported
	if (!supportedFeatures13.dynamicRendering || !supportedFeatures13.synchronization2 ||
		!supportedFeatures12.timelineSemaphore || !supportedFeatures12.descriptorIndexing ||
		!supportedFeatures12.descriptorBindingSampledImageUpdateAfterBind || !supportedFeatures12.descriptorBindingPartiallyBound ||
		!supportedFeatures12.runtimeDescriptorArray || !supportedFeatures.features.samplerAnisotropy ||
		!supportedFeatures.features.multiDrawIndirect || !supportedFeatures12.bufferDeviceAddress)
	{
		showError("Physical device doesn't meet the feature requirements");
		return false;
	}

	// produce a separate features struct chain for device creation
	VkPhysicalDeviceVulkan14Features features14
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
		.pNext = nullptr
	};
	VkPhysicalDeviceVulkan13Features features13
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &features14,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE,
	};
	VkPhysicalDeviceVulkan12Features features12
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &features13,
		.descriptorIndexing = VK_TRUE,
		.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
		.descriptorBindingPartiallyBound = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE,
		.scalarBlockLayout = VK_TRUE,
		.timelineSemaphore = VK_TRUE,
		.bufferDeviceAddress = VK_TRUE,
		//.bufferDeviceAddressCaptureReplay = (debugging) ? VK_TRUE : VK_FALSE
	};
	VkPhysicalDeviceFeatures2 features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &features12,
		.features
		{
			.multiDrawIndirect = VK_TRUE,
			.samplerAnisotropy = VK_TRUE,
			.shaderInt64 = VK_TRUE,
		}
	};

	// request the queues we'll be using
	std::vector<float> queuePriorities{ 1.0f };
	VkDeviceQueueCreateInfo gfxQueueInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = gfxQueueFamIdx,
		.queueCount = 1,
		.pQueuePriorities = queuePriorities.data()
	};

	// device specific extensions
	const std::vector<const char *> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo devCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &gfxQueueInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = nullptr // features struct chain is set in pNext
	};

	if (vkCreateDevice(physicalDevice, &devCreateInfo, nullptr, &device) != VK_SUCCESS)
	{
		return false;
	}

	// grab the VkQueue object finally
	vkGetDeviceQueue(device, gfxQueueFamIdx, 0, &gfxQueue);
	if (!gfxQueue)
	{
		showError("Couldn't get the graphics queue");
		return false;
	}
	return true;
}

bool VulkanRenderSystem::initializeVMA()
{
	VmaVulkanFunctions vmaFuncInfo{};
	VmaAllocatorCreateInfo vmaAllocInfo
	{
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = physicalDevice,
		.device = device,
		.pVulkanFunctions = &vmaFuncInfo,
		.instance = vulkanInstance,
		.vulkanApiVersion = VulkanVersion
	};

	// vma can import directly from volk
	vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);

	if (vmaCreateAllocator(&vmaAllocInfo, &vmaAllocator) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

bool VulkanRenderSystem::createSwapchain(uint32_t width, uint32_t height)
{
	// track swapchain size separate from window size
	swapchainWidth = width;
	swapchainHeight = height;

	// ensure we request an appropriate number of images
	VkSurfaceCapabilitiesKHR surfaceCaps{};
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps) != VK_SUCCESS)
	{
		showError("Couldn't get the surface capabilities");
		return false;
	}

	uint32_t requestedImageCount = std::max(2u, surfaceCaps.minImageCount);
	if (surfaceCaps.maxImageCount > 0)
	{
		requestedImageCount = std::min(requestedImageCount, surfaceCaps.maxImageCount);
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = requestedImageCount,
		.imageFormat = swapchainFormat,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent{.width = swapchainWidth, .height = swapchainHeight },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.preTransform = surfaceCaps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};

	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
	{
		showError("Error creating swapchain");
		return false;
	}

	// ask for the swapchain images
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
	swapchainImageViews.resize(imageCount);

	// create the swapchain image views
	for (size_t i = 0; i < swapchainImages.size(); ++i)
	{
		VkImageViewCreateInfo imgViewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchainFormat,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		if (vkCreateImageView(device, &imgViewInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
		{
			showError("Error creating swapchain image view");
			return false;
		}
	}

	return true;
}

void VulkanRenderSystem::destroySwapchain()
{
	for (VkImageView swapchainImgView : swapchainImageViews)
	{
		vkDestroyImageView(device, swapchainImgView, nullptr);
	}
	swapchainImageViews.clear();

	if (swapchain)
	{
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = nullptr;
	}
}

VkShaderModule VulkanRenderSystem::createShaderModule(const std::string &fileName, shaderc_shader_kind kind) const
{
	// read shader file from disk
	const std::string shaderPath = "sdl3-demo/engine/shaders/" + fileName;
	const std::string src = readTextFile(shaderPath);
	if (src.empty())
	{
		showError("Specified shader file doesn't exist: " + shaderPath);
		return nullptr;
	}

	// compile the shader to SPIR-V
	std::cout << "Compiling shader: " << shaderPath << std::endl;
	shaderc::Compiler compiler;
	shaderc::CompileOptions opts;
	opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
	opts.SetTargetSpirv(shaderc_spirv_version_1_6);
	opts.SetOptimizationLevel(shaderc_optimization_level_performance);
	shaderc::CompilationResult result = compiler.CompileGlslToSpv(src, kind, fileName.c_str(), opts);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cerr << "Shader Compilation Error: " << result.GetErrorMessage() << std::endl;
		return nullptr;
	}

	const size_t shaderSize = (result.cend() - result.cbegin()) * sizeof(uint32_t);
	// pass spir-v to vulkan and create shader-module
	VkShaderModuleCreateInfo moduleCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderSize,
		.pCode = result.cbegin()
	};

	VkShaderModule shaderModule = nullptr;
	if (vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		showError("Error creating shader module");
		return nullptr;
	}
	return shaderModule;
}

ShaderSet *VulkanRenderSystem::createShaders(const std::string &shaderName)
{
	auto shaderSet = std::make_unique<ShaderSet>();
	// create the shader modules that we'll need for the graphics pipeline
	shaderSet->vert = createShaderModule(std::format("{}.vert", shaderName), shaderc_vertex_shader);
	shaderSet->frag = createShaderModule(std::format("{}.frag", shaderName), shaderc_fragment_shader);
	ShaderSet *result = shaderSet.get();
	shaders.push_back(std::move(shaderSet));

	if (result->vert == nullptr || result->frag == nullptr)
	{
		return nullptr;
	}
	return result;
}

Pipeline VulkanRenderSystem::createGraphicsPipeline(const ShaderSet &shaderSet, const PipelineConfig &config) const
{
	// configure the shader stages struct
	const char *entryPoint = "main";
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages
	{
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderSet.vert,
			.pName = entryPoint
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = shaderSet.frag,
			.pName = entryPoint
		}
	};

	// vertex pulling, don't define vertex input details
	VkPipelineVertexInputStateCreateInfo vertInputInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};

	// input assembly, we'll be drawing triangle lists
	std::array<VkPrimitiveTopology, 2> topologyMap{
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = topologyMap[static_cast<size_t>(config.topology)]
	};

	// depth/stencil configuration
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.stencilTestEnable = VK_FALSE
	};

	// dynamic rendering allows to set this up...dynamically
	// we still need this struct though
	VkPipelineViewportStateCreateInfo viewportInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = nullptr,
		.scissorCount = 1,
		.pScissors = nullptr
	};

	// rasterizer settings
	VkPipelineRasterizationStateCreateInfo rasterInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		//.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1.0f
	};

	// No multisampling
	VkPipelineMultisampleStateCreateInfo multiSampleInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
	};

	// Alpha-blending (disabled for now), still need
	// attachment info and write mask
	VkPipelineColorBlendAttachmentState attachState
	{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo blendInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &attachState
	};

	// enable dynamic state
	std::vector<VkDynamicState> dynamicState
	{
		VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicState.size()),
		.pDynamicStates = dynamicState.data()
	};

	// structure required for dynamic rendering
	VkPipelineRenderingCreateInfo renderInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &swapchainFormat,
		.depthAttachmentFormat = depthFormat,
	};

	// need to define a pipeline layout
	VkPushConstantRange pushConstRange
	{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(DrawConstants)
	};

	std::array<VkDescriptorSetLayout, 2> dsLayouts{ globalDSLayout, frameDSLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = dsLayouts.size(),
		.pSetLayouts = dsLayouts.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstRange
	};

	VkPipelineLayout layout = nullptr;
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
	{
		showError("Unable to create the pipeline layout");
		return Pipeline{};
	}

	VkGraphicsPipelineCreateInfo pipelineInfo
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &renderInfo,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pViewportState = &viewportInfo,
		.pRasterizationState = &rasterInfo,
		.pMultisampleState = &multiSampleInfo,
		.pDepthStencilState = &depthStencilInfo,
		.pColorBlendState = &blendInfo,
		.pDynamicState = &dynamicStateInfo,
		.layout = layout,
		.renderPass = VK_NULL_HANDLE,
	};

	VkPipeline pipeline = nullptr;
	if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		showError("Error creating the pipeline");
		return Pipeline{};
	}
	return Pipeline{ .layout = layout, .handle = pipeline };
}

bool VulkanRenderSystem::createSyncResources()
{
	VkSemaphoreTypeCreateInfo semaphoreTypeInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = timelineValue
	};
	VkSemaphoreCreateInfo semaphoreInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &semaphoreTypeInfo
	};
	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &timelineSemaphore) != VK_SUCCESS)
	{
		showError("Unable to create the timeline semaphore");
		return false;
	}

	// per-frame image-acquire semaphores
	for (FrameResources &res : frameResources)
	{
		// create the binary semaphores
		VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &res.imageAcquiredSemaphore) != VK_SUCCESS)
		{
			showError("Error creating the per-frame image-acquire semaphore");
			return false;
		}
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &res.workCompleteSemaphore) != VK_SUCCESS)
		{
			showError("Error creating the per-frame image-acquire semaphore");
			return false;
		}
	}

	return true;
}

bool VulkanRenderSystem::createCommandBuffers()
{
	// create a command pool for single use
	VkCommandPoolCreateInfo poolInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = gfxQueueFamIdx
	};
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		showError("Unable to create command buffer pool");
		return false;
	}

	for (FrameResources &res : frameResources)
	{
		// we'll give each frame its own pool, faster cmd buffer resets this way
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = gfxQueueFamIdx
		};
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &res.commandPool) != VK_SUCCESS)
		{
			showError("Unable to create command buffer pool");
			return false;
		}

		// create the command buffer for this frame
		VkCommandBufferAllocateInfo cmdAllocInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = res.commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &res.commandBuffer) != VK_SUCCESS)
		{
			showError("Unable to allocate command buffer");
			return false;
		}
	}
	return true;
}

bool VulkanRenderSystem::createDescriptorSets()
{
	// create a pool to accomodate all descriptor sets
	std::array<VkDescriptorPoolSize, 2> poolSizes
	{
		VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = MaxTextures},
		VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1 * MaxFramesInFlight}
	};
	VkDescriptorPoolCreateInfo poolInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 3,
		.poolSizeCount = poolSizes.size(),
		.pPoolSizes = poolSizes.data()
	};
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descPool) != VK_SUCCESS)
	{
		showError("Unable to create descriptor pool");
		return false;
	}

	// global descriptor set
	{
		std::array<VkDescriptorSetLayoutBinding, 1> bindings =
		{
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = MaxTextures,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
			}
		};
		std::array<VkDescriptorBindingFlags, 1> flags;
		flags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = flags.size(),
			.pBindingFlags = flags.data()
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &flagsInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = bindings.size(),
			.pBindings = bindings.data()
		};

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &globalDSLayout) != VK_SUCCESS)
		{
			showError("Unable to create descriptor set layout");
			return false;
		}

		// create the actual descriptor sets
		VkDescriptorSetAllocateInfo descSetAllocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &globalDSLayout,
		};
		if (vkAllocateDescriptorSets(device, &descSetAllocInfo, &globalDescSet) != VK_SUCCESS)
		{
			showError("Unable to allocate descriptor set");
			return false;
		}
	}

	// frame descriptor set
	{
		std::array<VkDescriptorSetLayoutBinding, 1> bindings =
		{
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
			}
		};

		std::array<VkDescriptorBindingFlags, 1> flags;
		flags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = flags.size(),
			.pBindingFlags = flags.data()
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &flagsInfo,
			.flags = 0,
			.bindingCount = bindings.size(),
			.pBindings = bindings.data()
		};

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &frameDSLayout) != VK_SUCCESS)
		{
			showError("Unable to create descriptor set layout");
			return false;
		}

		// per-frame descriptor set creation
		VkDescriptorSetAllocateInfo descSetAllocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &frameDSLayout,
		};

		for (auto &res : frameResources)
		{
			if (vkAllocateDescriptorSets(device, &descSetAllocInfo, &res.descSet) != VK_SUCCESS)
			{
				showError("Unable to allocate descriptor set");
				return false;
			}
		}
	}

	return true;
}

Buffer VulkanRenderSystem::createBuffer(VkBufferUsageFlags usage, VkBufferCreateFlags flags, size_t byteSize, void *initData = nullptr)
{
	VkBufferCreateInfo buffInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = flags,
		.size = byteSize,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	VmaAllocationCreateInfo allocInfo
	{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};

	Buffer newBuff;
	if (vmaCreateBuffer(vmaAllocator, &buffInfo, &allocInfo, &newBuff.buffer, &newBuff.allocation, nullptr) != VK_SUCCESS)
	{
		showError("Error allocating buffer");
	}

	// write init data if provided
	if (initData)
	{
		void *buffPtr = nullptr;
		if (vmaMapMemory(vmaAllocator, newBuff.allocation, &buffPtr) != VK_SUCCESS)
		{
			showError("Unable to map buffer memory");
		}
		std::memcpy(static_cast<char *>(buffPtr), initData, buffInfo.size);
		const glm::vec3 *vec3Ptr = reinterpret_cast<const glm::vec3 *>(buffPtr);
		vmaUnmapMemory(vmaAllocator, newBuff.allocation);
	}

	return newBuff;
}

bool vks::VulkanRenderSystem::createInternalTargets()
{
	for (auto &res : frameResources)
	{
		VkImageCreateInfo imageInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = swapchainFormat,
			.extent {.width = logW, .height = logH, .depth = 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		VmaAllocationCreateInfo allocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

		if (vmaCreateImage(vmaAllocator, &imageInfo, &allocInfo, &res.renderTarget.handle, &res.renderTarget.allocation, nullptr) != VK_SUCCESS)
		{
			showError("Error internal render target image");
			return false;
		}

		VkImageViewCreateInfo imgViewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = res.renderTarget.handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchainFormat,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.levelCount = 1,
				.layerCount = 1
			}
		};

		if (vkCreateImageView(device, &imgViewInfo, nullptr, &res.renderTarget.view) != VK_SUCCESS)
		{
			showError("Error creating render target image view");
			return false;
		}
	}

	// create depth image
	VkImageCreateInfo depthCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depthFormat,
		.extent{.width = logW, .height = logH, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo allocInfo
	{
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};
	if (vmaCreateImage(vmaAllocator, &depthCreateInfo, &allocInfo, &depthImage, &depthImageAllocation, nullptr) != VK_SUCCESS)
	{
		showError("Error allocating depth image");
		return false;
	}

	VkImageViewCreateInfo depthImgViewInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = depthFormat,
		.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
	};
	if (vkCreateImageView(device, &depthImgViewInfo, nullptr, &depthImageView) != VK_SUCCESS)
	{
		showError("Error creating depth image view");
		return false;
	}

	return true;
}

bool vks::VulkanRenderSystem::createIndirectDrawBuffers()
{
	const size_t indirectBuffSize = MaxDrawCommands * sizeof(VkDrawIndexedIndirectCommand);
	const size_t instanceBuffSize = MaxInstances * sizeof(InstanceData);
	for (auto &res : frameResources)
	{
		// create buffer for the draw commands
		res.indirectDraws = createBuffer(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, 0, indirectBuffSize);
		if (res.indirectDraws.buffer == nullptr)
		{
			return false;
		}
		// buffer for instance data
		res.instanceData = createBuffer(VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT, 0, instanceBuffSize);
		if (res.instanceData.buffer == nullptr)
		{
			return false;
		}

		// keep both buffers mapped
		if (vmaMapMemory(vmaAllocator, res.indirectDraws.allocation, reinterpret_cast<void **>(&res.drawCommands)) != VK_SUCCESS)
		{
			return false;
		}
		if (vmaMapMemory(vmaAllocator, res.instanceData.allocation, reinterpret_cast<void **>(&res.instances)) != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

bool vks::VulkanRenderSystem::updatePerFrameDescriptors()
{
	for (auto &res : frameResources)
	{
		VkDescriptorBufferInfo dsWrite
		{
			.buffer = res.instanceData.buffer,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSet writes
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = res.descSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &dsWrite
		};
		vkUpdateDescriptorSets(device, 1, &writes, 0, nullptr);
	}
	return true;
}

void loadNode(tinygltf::Node &node, tinygltf::Model &model)
{
	using namespace tinygltf;

	for (int childNodeIndex : node.children)
	{
		loadNode(model.nodes[childNodeIndex], model);
	}
}

void VulkanRenderSystem::loadModel()
{
	using namespace tinygltf;
	Model model;

	tinygltf::LoadImageDataFunction imageLoaderFunc = [](tinygltf::Image *image, const int imgIndex, std::string *err, std::string *warn, int reqWidth, int reqHeight, const unsigned char *bytes, int size, void *userData) -> bool
	{
		return false;
	};

	TinyGLTF loader;
	loader.SetImageLoader(imageLoaderFunc, nullptr);

	std::string err;
	std::string warn;
	//loader.LoadASCIIFromFile(&model, &err, &warn, "C:/Users/nikol/Desktop/untitled.gltf");
	loader.LoadASCIIFromFile(&model, &err, &warn, "D:/glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
	//loader.LoadASCIIFromFile(&model, &err, &warn, "S:/projects/boiler-3d/data/sorceress/scene.gltf");

	// load images
	VkCommandBuffer commandBuffer = startTransientCommandBuffer();
	if (commandBuffer)
	{
		for (const tinygltf::Image &image : model.images)
		{
			auto [imageId, img] = createImage(image.width, image.height, image.component);
			if (!imageId.isValid())
			{
				showError("Image could not be loaded");
				break;
			}
		}
	}
	submitTransientCommandBuffer(commandBuffer);

	// load all meshes first
	for (const tinygltf::Mesh &mesh : model.meshes)
	{
		vks::Mesh newMesh;
		for (const Primitive &primitive : mesh.primitives)
		{
			SubMesh subMesh;
			subMesh.vertexStart = vertices.size();
			subMesh.indexStart = indices.size();

			// load primitive vertices into sub-mesh
			if (const auto &itr = primitive.attributes.find("POSITION"); itr != primitive.attributes.end())
			{
				const auto &[name, index] = *itr;
				const Accessor &access = model.accessors[index];
				const BufferView &bv = model.bufferViews[access.bufferView];
				const tinygltf::Buffer &buffer = model.buffers[bv.buffer];

				if (access.type == TINYGLTF_TYPE_VEC3)
				{
					for (int i = 0; i < access.count; ++i)
					{
						size_t offset = bv.byteOffset + access.byteOffset + i * ((bv.byteStride > 0) ? bv.byteStride : sizeof(glm::vec3));
						const glm::vec3 *pos = reinterpret_cast<const glm::vec3 *>(buffer.data.data() + offset);
						vertices.push_back(Vertex{ .position = *pos });
						subMesh.vertexCount++;
					}
				}
			}
			// load primitive vertices into sub-mesh
			if (const auto &itr = primitive.attributes.find("TEXCOORD_0"); itr != primitive.attributes.end())
			{
				const auto &[name, index] = *itr;
				const Accessor &access = model.accessors[index];
				const BufferView &bv = model.bufferViews[access.bufferView];
				const tinygltf::Buffer &buffer = model.buffers[bv.buffer];

				if (access.type == TINYGLTF_TYPE_VEC2)
				{
					for (int i = 0; i < access.count; ++i)
					{
						size_t offset = bv.byteOffset + access.byteOffset + i * ((bv.byteStride > 0) ? bv.byteStride : sizeof(glm::vec2));
						const glm::vec2 *uv = reinterpret_cast<const glm::vec2 *>(buffer.data.data() + offset);
						vertices[subMesh.vertexStart + i].uv = *uv;
					}
				}
			}
			// indices
			if (primitive.indices != -1)
			{
				const Accessor &access = model.accessors[primitive.indices];
				const BufferView &bv = model.bufferViews[access.bufferView];
				const tinygltf::Buffer &buffer = model.buffers[bv.buffer];
				subMesh.indexCount = access.count;

				if (access.type == TINYGLTF_TYPE_SCALAR && access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				{
					for (int i = 0; i < access.count; ++i)
					{
						const uint32_t *idx = reinterpret_cast<const uint32_t *>(buffer.data.data() + bv.byteOffset + access.byteOffset) + i;
						indices.push_back(*idx);
					}
				}
				else if (access.type == TINYGLTF_TYPE_SCALAR && access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					for (int i = 0; i < access.count; ++i)
					{
						const uint16_t *idx = reinterpret_cast<const uint16_t *>(buffer.data.data() + bv.byteOffset + access.byteOffset) + i;
						indices.push_back(*idx);
					}
				}
			}
			newMesh.subMeshes.push_back(subMesh);
		}
		meshes.push_back(newMesh);
	}

	auto createBuffer = [](VmaAllocator &vmaAllocator, VkBufferUsageFlags usage, size_t byteSize, void *initData)
	{
		VkBufferCreateInfo buffInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = byteSize,
			.usage = usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo allocInfo
		{
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
			.usage = VMA_MEMORY_USAGE_CPU_TO_GPU
		};

		vks::Buffer newBuff;
		if (vmaCreateBuffer(vmaAllocator, &buffInfo, &allocInfo, &newBuff.buffer, &newBuff.allocation, nullptr) != VK_SUCCESS)
		{
			//showError("Error allocating buffer");
		}

		void *buffPtr = nullptr;
		if (vmaMapMemory(vmaAllocator, newBuff.allocation, &buffPtr) != VK_SUCCESS)
		{
			//showError("Unable to map buffer memory");
		}
		std::memcpy(static_cast<char *>(buffPtr), initData, buffInfo.size);
		const glm::vec3 *vec3Ptr = reinterpret_cast<const glm::vec3 *>(buffPtr);
		vmaUnmapMemory(vmaAllocator, newBuff.allocation);

		return newBuff;
	};

	vertexBuffer = createBuffer(vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		sizeof(Vertex) * vertices.size(), vertices.data());
	indexBuffer = createBuffer(vmaAllocator, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		sizeof(uint32_t) * indices.size(), indices.data());
}

VkCommandBuffer VulkanRenderSystem::startTransientCommandBuffer()
{
	// allocate the transient command buffer
	VkCommandBufferAllocateInfo cmdAllocInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer = nullptr;
	if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &commandBuffer) != VK_SUCCESS)
	{
		showError("Unable to allocate command buffer");
		return nullptr;
	}

	// begin the command buffer
	VkCommandBufferBeginInfo beginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		showError("Unable to begin command buffer");
		return nullptr;
	}

	return commandBuffer;
}

void VulkanRenderSystem::submitTransientCommandBuffer(VkCommandBuffer commandBuffer, VkFence waitFence)
{
	vkEndCommandBuffer(commandBuffer);

	// TODO: Submit on a transfer queue
	VkSubmitInfo submitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	vkQueueSubmit(gfxQueue, 1, &submitInfo, waitFence);
	if (waitFence)
	{
		vkWaitForFences(device, 1, &waitFence, VK_TRUE, UINT64_MAX);
	}
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

std::tuple<ResourceId, Image> VulkanRenderSystem::createImage(uint32_t width, uint32_t height, uint32_t channels)
{
	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

	VkImageCreateInfo imageInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = imageFormat,
		.extent {.width = width, .height = height, .depth = 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VmaAllocationCreateInfo allocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

	Image image{ .width = width, .height = height, .channels = channels };
	if (vmaCreateImage(vmaAllocator, &imageInfo, &allocInfo, &image.handle, &image.allocation, nullptr) != VK_SUCCESS)
	{
		showError("Error creating image");
		return std::make_tuple(ResourceId{}, image);
	}

	VkImageViewCreateInfo imgViewInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image.handle,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = imageFormat,
		.subresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1
		}
	};

	if (vkCreateImageView(device, &imgViewInfo, nullptr, &image.view) != VK_SUCCESS)
	{
		showError("Error creating image view");
		return std::make_tuple(ResourceId{}, image);
	}

	// TODO: this needs to be better
	images.push_back(image);
	return std::make_tuple(ResourceId(images.size() - 1, ResourceId::Type::texture), image);
}

VkSampler VulkanRenderSystem::createSampler()
{
	VkSamplerCreateInfo samplerInfo
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = 16.0f, // Check limits
		.compareEnable = VK_FALSE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkSampler sampler = nullptr;
	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		showError("Unable to create texture sampler");
		return nullptr;
	}
	return sampler;
}

void VulkanRenderSystem::transitionImages(VkCommandBuffer commandBuffer, const std::span<Barrier> &barriers)
{
	const size_t maxTransitions = 5;
	assert(maxTransitions >= barriers.size());
	std::array<VkImageMemoryBarrier2, maxTransitions> vkBarriers;

	for (int i = 0; i < barriers.size(); ++i)
	{
		const Barrier &b = barriers[i];
		vkBarriers[i] =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = b.srcStageMask,
			.srcAccessMask = b.srcAccessMask,
			.dstStageMask = b.dstStageMask,
			.dstAccessMask = b.dstAccessMask,
			.oldLayout = b.oldLayout,
			.newLayout = b.newLayout,
			.image = b.image,
			.subresourceRange
			{
				.aspectMask = b.imageAspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
	}

	VkDependencyInfo depInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size()),
		.pImageMemoryBarriers = vkBarriers.data()
	};
	vkCmdPipelineBarrier2(commandBuffer, &depInfo);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderSystem::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
	}

	return VK_FALSE;
}

void VulkanRenderSystem::updateTextures()
{
	std::vector<VkDescriptorImageInfo> descriptorWrites;
	descriptorWrites.reserve(images.size());
	for (Image &img : images)
	{
		descriptorWrites.push_back({
			.sampler = nearestSampler,
			.imageView = img.view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			});
	}

	VkWriteDescriptorSet writes
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = globalDescSet,
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = static_cast<uint32_t>(descriptorWrites.size()),
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = descriptorWrites.data()
	};
	vkUpdateDescriptorSets(device, 1, &writes, 0, nullptr);
}

void VulkanRenderSystem::onEvent(NodeHandle target, const AnimationPlayEvent &event)
{
	Node &node = services.world().getNode(target);
	auto [sc] = getRequiredComponents(node);
	sc->setTexture(event.getTextureId());
}

void VulkanRenderSystem::onEvent(NodeHandle target, const AnimationStopEvent &event)
{
}

void VulkanRenderSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event)
{
	if (event.getDirection().x != 0)
	{
		Node &node = services.world().getNode(target);
		if (node.isLinkedWith(this))
		{
			auto [sc] = getRequiredComponents(node);
			sc->setFlipH(event.getDirection().x < 0 ? true : false);
		}
	}
}

ResourceId VulkanRenderSystem::loadTexture(const std::string &filepath, bool flipY)
{
	// get pixel data and image info
	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_set_flip_vertically_on_load(flipY);
	stbi_uc *pixData = stbi_load(filepath.c_str(), &width, &height, &channels, 4);

	auto [resId, img] = createImage(width, height, channels);
	if (!resId.isValid())
	{
		showError("Error creating texture image");
		if (pixData)
		{
			stbi_image_free(pixData);
		}
		return resId;
	}

	// fence needed to wait for transfer operations
	VkFenceCreateInfo fenceCreateInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	VkFence fence = nullptr;
	if (vkCreateFence(device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
	{
		showError("Unable to create texture transfer fence");
		return ResourceId{};
	}

	// create a staging buffer for image data CPU side
	Buffer stagingBuffer = createBuffer(VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, width * height * channels, pixData);
	stbi_image_free(pixData);

	VkCommandBuffer commandBuffer = startTransientCommandBuffer();
	if (commandBuffer)
	{
		// transition the image as a transfer dst
		std::array<Barrier, 1> barrierTransfer = {
			Barrier {
				.image = img.handle,
				.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
				.srcAccessMask = 0,
				.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			}
		};
		transitionImages(commandBuffer, barrierTransfer);

		// copy the image data
		VkBufferImageCopy2 buffImgCopy
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
			.imageSubresource {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .layerCount = 1 },
			.imageExtent {.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height), .depth = 1}
		};
		VkCopyBufferToImageInfo2 copyBuffToImg
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.srcBuffer = stagingBuffer.buffer,
			.dstImage = img.handle,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &buffImgCopy
		};
		vkCmdCopyBufferToImage2(commandBuffer, &copyBuffToImg);

		// transition the image for sampling
		std::array<Barrier, 1> barrierColor =
		{
			Barrier {
				.image = img.handle,
				.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			}
		};
		transitionImages(commandBuffer, barrierColor);
	}
	submitTransientCommandBuffer(commandBuffer, fence);
	vmaDestroyBuffer(vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);
	if (fence)
	{
		vkDestroyFence(device, fence, nullptr);
	}

	return resId;
}

