#include "VulkanRenderer.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanImage.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanShader.hpp"

#include "Resources/Resources.hpp"
#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"
#include <Containers/Vector.hpp>
#include "Math/Math.hpp"
#include "Core/Events.hpp"
#include "Core/Settings.hpp"

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* user_data)
{
	switch (messageSeverity)
	{
	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		Logger::Error(callbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Logger::Warn(callbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		Logger::Info(callbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		Logger::Trace(callbackData->pMessage); break;
	}

	return VK_FALSE;
}

static RendererState* rendererState;
static U32 cachedFramebufferWidth = 0;
static U32 cachedFramebufferHeight = 0;

bool VulkanRenderer::Initialize(const String& applicationName, U8& renderTargetCount)
{
	rendererState = (RendererState*)Memory::Allocate(sizeof(RendererState), MEMORY_TAG_RENDERER);
	rendererState->FindMemoryIndex = FindMemoryIndex;
	rendererState->device = (VulkanDevice*)Memory::Allocate(sizeof(VulkanDevice), MEMORY_TAG_RENDERER);
	rendererState->swapchain = (VulkanSwapchain*)Memory::Allocate(sizeof(VulkanSwapchain), MEMORY_TAG_RENDERER);
	rendererState->objectIndexBuffer = (VulkanBuffer*)Memory::Allocate(sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);
	rendererState->objectVertexBuffer = (VulkanBuffer*)Memory::Allocate(sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);

	OnResize();

	rendererState->allocator = nullptr;

	if (!CreateInstance(applicationName) || !CreateDebugger() || !CreateSurface()) { return false; }

	rendererState->device->Create(rendererState);
	rendererState->swapchain->Create(rendererState, rendererState->framebufferWidth, rendererState->framebufferHeight);
	rendererState->framebufferSizeLastGeneration = 0;
	rendererState->framebufferSizeGeneration = 0;

	renderTargetCount = rendererState->swapchain->imageCount;

	CreateCommandBuffers();

	CreateSyncObjects();

	CreateBuffers();

	return true;
}

void VulkanRenderer::Shutdown()
{
	vkDeviceWaitIdle(rendererState->device->logicalDevice);

	Logger::Info("Destroying vulkan buffers...");
	rendererState->objectIndexBuffer->Destroy(rendererState);
	rendererState->objectVertexBuffer->Destroy(rendererState);
	Memory::Free(rendererState->objectIndexBuffer, sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);
	Memory::Free(rendererState->objectVertexBuffer, sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);

	Logger::Info("Destroying vulkan sync objects...");

	for (U8 i = 0; i < rendererState->swapchain->maxFramesInFlight; ++i)
	{
		if (rendererState->imageAvailableSemaphores[i])
		{
			vkDestroySemaphore(rendererState->device->logicalDevice, rendererState->imageAvailableSemaphores[i], rendererState->allocator);
			rendererState->imageAvailableSemaphores[i] = nullptr;
		}
		if (rendererState->queueCompleteSemaphores[i])
		{
			vkDestroySemaphore(rendererState->device->logicalDevice, rendererState->queueCompleteSemaphores[i], rendererState->allocator);
			rendererState->queueCompleteSemaphores[i] = nullptr;
		}
		vkDestroyFence(rendererState->device->logicalDevice, rendererState->inFlightFences[i], rendererState->allocator);
	}

	Logger::Info("Destroying vulkan commandbuffers...");
	for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
	{
		if (rendererState->graphicsCommandBuffers[i].handle)
		{
			rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
			rendererState->graphicsCommandBuffers[i].handle = nullptr;
		}
	}

	rendererState->swapchain->Destroy(rendererState);
	Memory::Free(rendererState->swapchain, sizeof(VulkanSwapchain), MEMORY_TAG_RENDERER);

	rendererState->device->Destroy(rendererState);
	Memory::Free(rendererState->device, sizeof(VulkanDevice), MEMORY_TAG_RENDERER);

	vkDestroySurfaceKHR(rendererState->instance, rendererState->surface, rendererState->allocator);
#ifdef NH_DEBUG
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(rendererState->instance, "vkDestroyDebugUtilsMessengerEXT");
	func(rendererState->instance, rendererState->debugMessenger, rendererState->allocator);
#endif
	vkDestroyInstance(rendererState->instance, rendererState->allocator);

	rendererState->graphicsCommandBuffers.Destroy();
	rendererState->imageAvailableSemaphores.Destroy();
	rendererState->queueCompleteSemaphores.Destroy();

	Memory::Free(rendererState, sizeof(RendererState), MEMORY_TAG_RENDERER);
}

void* VulkanRenderer::operator new(U64 size)
{
	return Memory::Allocate(size, MEMORY_TAG_RENDERER);
}

void VulkanRenderer::operator delete(void* p)
{
	Memory::Free(p, sizeof(VulkanRenderer), MEMORY_TAG_RENDERER);
}

bool VulkanRenderer::CreateInstance(const String& applicationName)
{
	Logger::Info("Creating vulkan instance...");

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(0, 2, 0);
	appInfo.pApplicationName = (const char*)applicationName;
	appInfo.pEngineName = "Nihility";
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.pApplicationInfo = &appInfo;

	Vector<const char*> extentionNames;
	extentionNames.Push(VK_KHR_SURFACE_EXTENSION_NAME);
	GetPlatformExtentions(extentionNames);

#ifdef NH_DEBUG
	extentionNames.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	Vector<const char*> layerNames;
	layerNames.Push("VK_LAYER_KHRONOS_validation");

	U32 availableLayerCount;
	VkCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
	Vector<VkLayerProperties> availableLayers(availableLayerCount, {});
	VkCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.Data()));

	for (U32 i = 0; i < layerNames.Size(); ++i)
	{
		bool found = false;
		for (U32 j = 0; j < availableLayerCount; ++j)
		{
			if (strcmp(layerNames[i], availableLayers[j].layerName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			Logger::Fatal("Required validation layer is missing: {}", layerNames[i]);
			return false;
		}
	}

	instanceInfo.enabledLayerCount = (U32)layerNames.Size();
	instanceInfo.ppEnabledLayerNames = layerNames.Data();
#else
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;
#endif

	instanceInfo.enabledExtensionCount = (U32)extentionNames.Size();
	instanceInfo.ppEnabledExtensionNames = extentionNames.Data();

	instanceInfo.flags = NULL;
	instanceInfo.pNext = nullptr;

	return vkCreateInstance(&instanceInfo, rendererState->allocator, &rendererState->instance) == VK_SUCCESS;
}

bool VulkanRenderer::CreateDebugger()
{
#ifdef NH_DEBUG
	Logger::Debug("Creating vulkan debugger...");
	U32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
	//    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

	VkDebugUtilsMessengerCreateInfoEXT debugInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugInfo.messageSeverity = logSeverity;
	debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugInfo.pfnUserCallback = VkDebugCallback;

	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(rendererState->instance, "vkCreateDebugUtilsMessengerEXT");
	ASSERT_MSG(func, "Failed to create debug messenger!");
	return func(rendererState->instance, &debugInfo, rendererState->allocator, &rendererState->debugMessenger) == VK_SUCCESS;
#else
	return true;
#endif
}

bool VulkanRenderer::CreateSurface()
{
	Logger::Info("Creating vulkan surface...");

#ifdef PLATFORM_WINDOWS
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.pNext = nullptr;
	surfaceInfo.flags = NULL;
	Platform::GetVulkanSurfaceInfo(&surfaceInfo.flags + sizeof(VkWin32SurfaceCreateFlagsKHR)); //TODO: Please don't do this
	return vkCreateWin32SurfaceKHR(rendererState->instance, &surfaceInfo, rendererState->allocator, &rendererState->surface) == VK_SUCCESS;
#elif PLATFORM_LINUX
	//TODO:
#elif PLATFORM_APPLE
	//TODO:
#endif
	return false;
}

void VulkanRenderer::CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext)
{
	renderpass->internalData = Memory::Allocate(sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);
	VulkanRenderpass* vulkanRenderpass = (VulkanRenderpass*)renderpass->internalData;
	vulkanRenderpass->hasPrevPass = hasPrev;
	vulkanRenderpass->hasNextPass = hasNext;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// Attachments TODO: make this configurable.
	Vector<VkAttachmentDescription> attachmentDescriptions;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = rendererState->swapchain->imageFormat.format;  // TODO: configurable
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = (renderpass->clearFlags & RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) ? VK_ATTACHMENT_LOAD_OP_CLEAR :
		hasPrev ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = hasPrev ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

	colorAttachment.finalLayout = hasNext ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachment.flags = 0;

	attachmentDescriptions.Push(colorAttachment);

	VkAttachmentReference colorAttachmentReference;
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;

	VkAttachmentReference depthAttachmentReference;
	VkAttachmentDescription depthAttachment{};
	if (renderpass->clearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG)
	{
		depthAttachment.format = rendererState->device->depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachmentDescriptions.Push(depthAttachment);

		depthAttachmentReference.attachment = 1;
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// TODO: other attachment types (input, resolve, preserve)

		subpass.pDepthStencilAttachment = &depthAttachmentReference;
	}
	else
	{
		subpass.pDepthStencilAttachment = nullptr;
	}

	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.pResolveAttachments = nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	// Render pass dependencies. TODO: make this configurable.
	VkSubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	VkRenderPassCreateInfo renderpassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderpassInfo.attachmentCount = (U32)attachmentDescriptions.Size();
	renderpassInfo.pAttachments = attachmentDescriptions.Data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &dependency;
	renderpassInfo.pNext = nullptr;
	renderpassInfo.flags = 0;

	VkCheck(vkCreateRenderPass(rendererState->device->logicalDevice, &renderpassInfo, rendererState->allocator, &vulkanRenderpass->handle));

	for (U8 i = 0; i < renderpass->targets.Size(); ++i)
	{
		Texture* windowTargetTexture = GetWindowAttachment(i);

		Vector<Texture*> attachments;
		attachments.Push(windowTargetTexture);

		if (renderpass->clearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG)
		{
			Texture* depthTargetTexture = GetDepthAttachment();
			attachments.Push(depthTargetTexture);
		}

		if (!CreateRenderTarget(attachments, renderpass, rendererState->framebufferWidth, rendererState->framebufferHeight, &renderpass->targets[i])) { return; }
	}
}

void VulkanRenderer::DestroyRenderpass(Renderpass* renderpass)
{
	if (renderpass && renderpass->internalData)
	{
		vkDeviceWaitIdle(rendererState->device->logicalDevice);

		for (U8 i = 0; i < renderpass->targets.Size(); ++i)
		{
			DestroyRenderTarget(&renderpass->targets[i], false);
		}

		VulkanRenderpass* vulkanRenderpass = (VulkanRenderpass*)renderpass->internalData;
		vkDestroyRenderPass(rendererState->device->logicalDevice, vulkanRenderpass->handle, rendererState->allocator);
		vulkanRenderpass->handle = nullptr;
		Memory::Free(vulkanRenderpass, sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);
		renderpass->internalData = nullptr;
	}
}

void VulkanRenderer::CreateCommandBuffers()
{
	Logger::Info("Creating vulkan command buffers...");

	if (!rendererState->graphicsCommandBuffers.Size())
	{
		rendererState->graphicsCommandBuffers.Resize(rendererState->swapchain->imageCount);
		for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
		{
			Memory::Zero(&rendererState->graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
		}
	}

	for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
	{
		if (rendererState->graphicsCommandBuffers[i].handle)
		{
			rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
		}

		Memory::Zero(&rendererState->graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
		rendererState->graphicsCommandBuffers[i].Allocate(rendererState, rendererState->device->graphicsCommandPool, true);
	}
}

void VulkanRenderer::CreateSyncObjects()
{
	rendererState->imageAvailableSemaphores.Resize(rendererState->swapchain->maxFramesInFlight);
	rendererState->queueCompleteSemaphores.Resize(rendererState->swapchain->maxFramesInFlight);

	for (U8 i = 0; i < rendererState->swapchain->maxFramesInFlight; ++i)
	{
		VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(rendererState->device->logicalDevice, &semaphoreInfo, rendererState->allocator, &rendererState->imageAvailableSemaphores[i]);
		vkCreateSemaphore(rendererState->device->logicalDevice, &semaphoreInfo, rendererState->allocator, &rendererState->queueCompleteSemaphores[i]);

		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VkCheck(vkCreateFence(rendererState->device->logicalDevice, &fenceInfo, rendererState->allocator, &rendererState->inFlightFences[i]));
	}

	for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
	{
		rendererState->imagesInFlight[i] = 0;
	}
}

void VulkanRenderer::GetPlatformExtentions(Vector<const char*>& names)
{
#ifdef PLATFORM_WINDOWS
	names.Push("VK_KHR_win32_surface");
#elif PLATFORM_LINUX
	names.Push("VK_KHR_xcb_surface");
#elif PLATFORM_APPLE
	names.Push("VK_EXT_metal_surface");
#endif
}

I32 VulkanRenderer::FindMemoryIndex(U32 memoryTypeBits, VkMemoryPropertyFlags memoryFlags)
{
	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(rendererState->device->physicalDevice, &properties);

	for (U32 i = 0; i < properties.memoryTypeCount; ++i)
	{
		if (memoryTypeBits & (1 << i) && (properties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags) { return i; }
	}

	Logger::Error("Unable to find suitable memory type!");
	return -1;
}

bool VulkanRenderer::CreateBuffers()
{
	Logger::Info("Creating vulkan buffers...");

	VkMemoryPropertyFlagBits memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	//TODO: Make this dynamic
	const U64 vertexBufferSize = sizeof(Vertex) * 1024 * 1024;

	if (!rendererState->objectVertexBuffer->Create(rendererState, vertexBufferSize,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
		memoryPropertyFlags, true, true))
	{
		Logger::Fatal("Error creating vertex buffer.");
		return false;
	}

	const U64 indexBufferSize = sizeof(U32) * 1024 * 1024;
	if (!rendererState->objectIndexBuffer->Create(rendererState, indexBufferSize,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
		memoryPropertyFlags, true, true))
	{
		Logger::Fatal("Error creating vertex buffer.");
		return false;
	}

	return true;
}

bool VulkanRenderer::BeginFrame()
{
	if (rendererState->recreatingSwapchain)
	{
		VkCheck_ERROR(vkDeviceWaitIdle(rendererState->device->logicalDevice));

		Logger::Info("Recreating swapchain, booting.");
		return false;
	}

	if (rendererState->framebufferSizeGeneration != rendererState->framebufferSizeLastGeneration)
	{
		VkCheck_ERROR(vkDeviceWaitIdle(rendererState->device->logicalDevice));

		if (!RecreateSwapchain()) { return false; }

		Logger::Info("Resized, booting.");
		return false;
	}

	VkCheck_FATAL(vkWaitForFences(rendererState->device->logicalDevice, 1, &rendererState->inFlightFences[rendererState->currentFrame], true, UINT64_MAX));

	if (!rendererState->swapchain->AcquireNextImageIndex(rendererState, UINT64_MAX,
		rendererState->imageAvailableSemaphores[rendererState->currentFrame], 0, &rendererState->imageIndex))
	{
		Logger::Error("Failed to acquire next image index, booting.");
		return false;
	}

	VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];
	commandBuffer->Reset();
	commandBuffer->Begin(false, false, false);

	VkViewport viewport;
	viewport.x = (F32)rendererState->renderArea.x;
	viewport.y = (F32)rendererState->renderArea.y;
	viewport.width = (F32)rendererState->renderArea.z;
	viewport.height = (F32)rendererState->renderArea.w;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset.x = rendererState->renderArea.x;
	scissor.offset.y = rendererState->renderArea.y;
	scissor.extent.width = rendererState->renderArea.z;
	scissor.extent.height = rendererState->renderArea.w;

	vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

	return true;
}

bool VulkanRenderer::EndFrame()
{
	VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];

	commandBuffer->End();

	if (rendererState->imagesInFlight[rendererState->imageIndex] != VK_NULL_HANDLE)
	{
		VkCheck_FATAL(vkWaitForFences(rendererState->device->logicalDevice, 1, &rendererState->imagesInFlight[rendererState->imageIndex], true, UINT64_MAX));
	}

	rendererState->imagesInFlight[rendererState->imageIndex] = rendererState->inFlightFences[rendererState->currentFrame];

	VkCheck(vkResetFences(rendererState->device->logicalDevice, 1, &rendererState->inFlightFences[rendererState->currentFrame]));

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->handle;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &rendererState->queueCompleteSemaphores[rendererState->currentFrame];

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &rendererState->imageAvailableSemaphores[rendererState->currentFrame];

	VkPipelineStageFlags flags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = flags;

	VkCheck_ERROR(vkQueueSubmit(rendererState->device->graphicsQueue, 1,
		&submitInfo, rendererState->inFlightFences[rendererState->currentFrame]));

	commandBuffer->UpdateSubmitted();

	rendererState->swapchain->Present(rendererState, rendererState->device->graphicsQueue, rendererState->device->presentQueue,
		rendererState->queueCompleteSemaphores[rendererState->currentFrame], rendererState->imageIndex);

	return true;
}

bool VulkanRenderer::BeginRenderpass(Renderpass* renderpass)
{
	VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];

	VulkanRenderpass* vulkanRenderpass = (VulkanRenderpass*)renderpass->internalData;
	RenderTarget& target = renderpass->targets[rendererState->imageIndex];

	VkRenderPassBeginInfo beginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	beginInfo.renderPass = vulkanRenderpass->handle;
	beginInfo.framebuffer = (VkFramebuffer)target.internalFramebuffer;
	beginInfo.renderArea.offset.x = (I32)(rendererState->renderArea.x);
	beginInfo.renderArea.offset.y = (I32)(rendererState->renderArea.y);
	beginInfo.renderArea.extent.width = (U32)(rendererState->renderArea.z);
	beginInfo.renderArea.extent.height = (U32)(rendererState->renderArea.w);

	Vector<VkClearValue> clearValues;
	VkClearValue colorClear{};
	if (renderpass->clearFlags & RENDERPASS_CLEAR_COLOR_BUFFER_FLAG)
	{
		Memory::Copy(colorClear.color.float32, &renderpass->clearColor, sizeof(Vector4));
	}

	clearValues.Push(colorClear);

	VkClearValue depthClear{};
	if (renderpass->clearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG)
	{
		depthClear.depthStencil.depth = renderpass->depth;
		depthClear.depthStencil.stencil = (renderpass->clearFlags & RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG) ? renderpass->stencil : 0;
		clearValues.Push(depthClear);
	}

	beginInfo.clearValueCount = (U32)clearValues.Size();
	beginInfo.pClearValues = clearValues.Data();

	vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;

	return true;
}

bool VulkanRenderer::EndRenderpass(Renderpass* renderpass)
{
	VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];

	vkCmdEndRenderPass(commandBuffer->handle);
	commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
	return true;
}

bool VulkanRenderer::CreateMesh(Mesh* mesh)
{
	if (mesh->vertices.Size() < 3)
	{
		Logger::Error("CreateMesh requires at least three vertices, vertex count provided: {}", mesh->vertices.Size());
		return false;
	}

	VulkanMesh* internalData = (VulkanMesh*)mesh->internalData;

	if (internalData)
	{
		FreeDataRange(rendererState->objectVertexBuffer, internalData->vertexBufferOffset, internalData->vertexElementSize * internalData->vertexCount);

		if (internalData->indexElementSize > 0)
		{
			FreeDataRange(rendererState->objectIndexBuffer, internalData->indexBufferOffset, internalData->indexElementSize * internalData->indexCount);
			internalData->indexCount = 0;
		}
	}
	else
	{
		internalData = (VulkanMesh*)Memory::Allocate(sizeof(VulkanMesh), MEMORY_TAG_RESOURCE);
		mesh->internalData = internalData;
	}

	VkCommandPool pool = rendererState->device->graphicsCommandPool;
	VkQueue queue = rendererState->device->graphicsQueue;

	internalData->vertexCount = (U32)mesh->vertices.Size();
	internalData->vertexElementSize = sizeof(Vertex);
	U32 totalSize = (U32)mesh->vertices.Size() * sizeof(Vertex);

	if (!UploadDataRange(pool, nullptr, queue, rendererState->objectVertexBuffer, internalData->vertexBufferOffset, totalSize, mesh->vertices.Data()))
	{
		Logger::Error("CreateMesh: Failed to upload to the vertex buffer!");
		return false;
	}

	if (mesh->indices.Size())
	{
		internalData->indexCount = (U32)mesh->indices.Size();
		internalData->indexElementSize = sizeof(U32);
		totalSize = (U32)mesh->indices.Size() * sizeof(U32);
		if (!UploadDataRange(pool, nullptr, queue, rendererState->objectIndexBuffer, internalData->indexBufferOffset, totalSize, mesh->indices.Data()))
		{
			Logger::Error("CreateMesh: Failed to upload to the index buffer!");
			return false;
		}
	}

	++internalData->generation;

	return true;
}

void VulkanRenderer::DestroyMesh(Mesh* mesh)
{
	if (mesh && mesh->internalData)
	{
		vkDeviceWaitIdle(rendererState->device->logicalDevice);
		VulkanMesh* internalData = (VulkanMesh*)mesh->internalData;

		FreeDataRange(rendererState->objectVertexBuffer, internalData->vertexBufferOffset, internalData->vertexElementSize * internalData->vertexCount);

		if (internalData->indexElementSize > 0)
		{
			FreeDataRange(rendererState->objectIndexBuffer, internalData->indexBufferOffset, internalData->indexElementSize * internalData->indexCount);
		}

		Memory::Free(internalData, sizeof(VulkanMesh), MEMORY_TAG_RESOURCE);
		internalData->id = INVALID_ID;
		internalData->generation = INVALID_ID;
	}
}

void VulkanRenderer::DrawMesh(const MeshRenderData& meshdata)
{
	if (meshdata.mesh && !meshdata.mesh->internalData) { return; }

	VulkanMesh* bufferData = (VulkanMesh*)meshdata.mesh->internalData;
	VulkanCommandBuffer& commandBuffer = rendererState->graphicsCommandBuffers[rendererState->imageIndex];

	VkDeviceSize offsets[1] = { bufferData->vertexBufferOffset };
	vkCmdBindVertexBuffers(commandBuffer.handle, 0, 1, &rendererState->objectVertexBuffer->handle, (VkDeviceSize*)offsets);

	if (bufferData->indexCount > 0)
	{
		vkCmdBindIndexBuffer(commandBuffer.handle, rendererState->objectIndexBuffer->handle, bufferData->indexBufferOffset, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer.handle, bufferData->indexCount, 1, 0, 0, 0);
	}
	else
	{
		vkCmdDraw(commandBuffer.handle, bufferData->vertexCount, 1, 0, 0);
	}
}

void VulkanRenderer::OnResize()
{
	rendererState->framebufferWidth = Settings::WindowWidth;
	rendererState->framebufferHeight = Settings::WindowHeight;
	++rendererState->framebufferSizeGeneration;

	F32 aspectHeight = Settings::WindowHeight * 1.77777777778f;
	F32 aspectWidth = Settings::WindowWidth * 0.5625f;

	if (Settings::WindowWidth > aspectHeight)
	{
		F32 offset = (Settings::WindowWidth - aspectHeight) * 0.5f;

		rendererState->renderArea.x = (I32)offset;
		rendererState->renderArea.y = 0;
		rendererState->renderArea.z = (I32)(Settings::WindowWidth - (offset * 2.0f));
		rendererState->renderArea.w = Settings::WindowHeight;
	}
	else
	{
		F32 offset = (Settings::WindowHeight - aspectWidth) * 0.5f;

		rendererState->renderArea.x = 0;
		rendererState->renderArea.y = (I32)offset;
		rendererState->renderArea.z = Settings::WindowWidth;
		rendererState->renderArea.w = (I32)(Settings::WindowHeight - (offset * 2.0f));
	}
}

Vector2Int VulkanRenderer::WindowSize()
{
	return { rendererState->renderArea.z, rendererState->renderArea.w };
}

void VulkanRenderer::CreateTexture(Texture* texture, const Vector<U8>& pixels)
{
	texture->internalData = (VulkanImage*)Memory::Allocate(sizeof(VulkanImage), MEMORY_TAG_RESOURCE);
	VulkanImage* image = (VulkanImage*)texture->internalData;

	switch (texture->layout)
	{
	case IMAGE_LAYOUT_RGBA32: image->format = VK_FORMAT_R8G8B8A8_UNORM; break;
	case IMAGE_LAYOUT_BGRA32: image->format = VK_FORMAT_B8G8R8A8_UNORM; break;
	case IMAGE_LAYOUT_RGB24: image->format = VK_FORMAT_R8G8B8_UNORM; break;
	case IMAGE_LAYOUT_BGR24: image->format = VK_FORMAT_B8G8R8_UNORM; break;
	}

	// NOTE: Lots of assumptions here, different texture types will require
	// different options here.
	image->Create(
		rendererState,
		VK_IMAGE_TYPE_2D,
		texture->width,
		texture->height,
		image->format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		VK_IMAGE_ASPECT_COLOR_BIT);

	WriteTextureData(texture, pixels);

	++texture->generation;
}

void VulkanRenderer::DestroyTexture(Texture* texture)
{
	vkDeviceWaitIdle(rendererState->device->logicalDevice);

	VulkanImage* image = (VulkanImage*)texture->internalData;
	if (image)
	{
		image->Destroy(rendererState);

		Memory::Free(texture->internalData, sizeof(VulkanImage), MEMORY_TAG_RESOURCE);
		texture->internalData = nullptr;
	}
}

bool VulkanRenderer::CreateWritableTexture(Texture* texture)
{
	texture->internalData = (VulkanImage*)Memory::Allocate(sizeof(VulkanImage), MEMORY_TAG_RESOURCE);
	VulkanImage* image = (VulkanImage*)texture->internalData;

	switch (texture->layout)
	{
	case IMAGE_LAYOUT_RGBA32: image->format = VK_FORMAT_R8G8B8A8_UNORM; break;
	case IMAGE_LAYOUT_BGRA32: image->format = VK_FORMAT_B8G8R8A8_UNORM; break;
	case IMAGE_LAYOUT_RGB24: image->format = VK_FORMAT_R8G8B8_UNORM; break;
	case IMAGE_LAYOUT_BGR24: image->format = VK_FORMAT_B8G8R8_UNORM; break;
	}

	// NOTE: Lots of assumptions here, different texture types will require
	// different options here.
	if (!image->Create(
		rendererState,
		VK_IMAGE_TYPE_2D,
		texture->width,
		texture->height,
		image->format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		VK_IMAGE_ASPECT_COLOR_BIT))
	{
		Memory::Free(texture->internalData, sizeof(VulkanImage), MEMORY_TAG_RESOURCE);
		texture->internalData = nullptr;
		return false;
	}

	++texture->generation;

	return true;
}

void VulkanRenderer::WriteTextureData(Texture* texture, const Vector<U8>& pixels)
{
	VulkanImage* image = (VulkanImage*)texture->internalData;

	VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VulkanBuffer staging;
	staging.Create(rendererState, pixels.Size(), usage, memoryPropertyFlags, true, false);
	staging.LoadData(rendererState, 0, pixels.Size(), 0, pixels.Data());

	VulkanCommandBuffer tempBuffer;
	VkCommandPool pool = rendererState->device->graphicsCommandPool;
	VkQueue queue = rendererState->device->graphicsQueue;
	tempBuffer.AllocateAndBeginSingleUse(rendererState, pool);

	image->TransitionLayout(
		rendererState,
		&tempBuffer,
		image->format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	image->CopyFromBuffer(rendererState, staging.handle, &tempBuffer);

	image->TransitionLayout(
		rendererState,
		&tempBuffer,
		image->format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	tempBuffer.EndSingleUse(rendererState, pool, queue);

	staging.Destroy(rendererState);

	++texture->generation;
}

void VulkanRenderer::ResizeTexture(Texture* texture, U32 width, U32 height)
{
	texture->internalData = (VulkanImage*)Memory::Allocate(sizeof(VulkanImage), MEMORY_TAG_RESOURCE);
	VulkanImage* image = (VulkanImage*)texture->internalData;

	// NOTE: Lots of assumptions here, different texture types will require
	// different options here.
	image->Create(
		rendererState,
		VK_IMAGE_TYPE_2D,
		width,
		height,
		image->format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		VK_IMAGE_ASPECT_COLOR_BIT);

	++texture->generation;
}

bool VulkanRenderer::AcquireTextureMapResources(TextureMap& map)
{
	VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

	samplerInfo.minFilter = ConvertFilterType(map.filterMinify);
	samplerInfo.magFilter = ConvertFilterType(map.filterMagnify);

	samplerInfo.addressModeU = ConvertRepeatType(map.repeatU);
	samplerInfo.addressModeV = ConvertRepeatType(map.repeatV);
	samplerInfo.addressModeW = ConvertRepeatType(map.repeatW);

	// TODO: Configurable
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler sampler = (VkSampler)map.internalData;

	VkCheck_ERROR(vkCreateSampler(rendererState->device->logicalDevice, &samplerInfo, rendererState->allocator, &sampler));

	map.internalData = sampler;

	return true;
}

void VulkanRenderer::ReleaseTextureMapResources(TextureMap& map)
{
	vkDeviceWaitIdle(rendererState->device->logicalDevice);
	vkDestroySampler(rendererState->device->logicalDevice, (VkSampler)map.internalData, rendererState->allocator);
	map.internalData = nullptr;
}

VkFilter VulkanRenderer::ConvertFilterType(TextureFilter filter)
{
	switch (filter)
	{
	case TEXTURE_FILTER_MODE_NEAREST: return VK_FILTER_NEAREST;
	case TEXTURE_FILTER_MODE_LINEAR:  return VK_FILTER_LINEAR;
	default:
		Logger::Warn("ConvertFilterType: Unsupported filter type '{}', defaulting to linear...", (I32)filter);
		return VK_FILTER_LINEAR;
	}
}

VkSamplerAddressMode VulkanRenderer::ConvertRepeatType(TextureRepeat repeat)
{
	switch (repeat)
	{
	case TEXTURE_REPEAT_REPEAT:             return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case TEXTURE_REPEAT_MIRRORED_REPEAT:    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case TEXTURE_REPEAT_CLAMP_TO_EDGE:      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case TEXTURE_REPEAT_CLAMP_TO_BORDER:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	default:
		Logger::Warn("ConvertRepeatType: Type '{}' not supported, defaulting to repeat...", repeat);
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

bool VulkanRenderer::CreateShader(Shader* shader)
{
	if (!shader->internalData)
	{
		shader->internalData = Memory::Allocate(sizeof(VulkanShader), MEMORY_TAG_RESOURCE);
		VulkanShader* outShader = (VulkanShader*)shader->internalData;
		if (outShader) { return outShader->Create(rendererState, shader); }
	}

	return false;
}

void VulkanRenderer::DestroyShader(Shader* shader)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { outShader->Destroy(rendererState); }
	Memory::Free(shader->internalData, sizeof(VulkanShader), MEMORY_TAG_RESOURCE);
}

bool VulkanRenderer::InitializeShader(Shader* shader)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { return outShader->Initialize(rendererState, shader); }
	return false;
}

bool VulkanRenderer::UseShader(Shader* shader)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { return outShader->Use(rendererState); }
	return false;
}

bool VulkanRenderer::ApplyGlobals(Shader* shader)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { return outShader->ApplyGlobals(rendererState, shader); }
	return false;
}

bool VulkanRenderer::ApplyInstance(Shader* shader, bool needsUpdate)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { return outShader->ApplyInstance(rendererState, shader, needsUpdate); }
	return false;
}

U32 VulkanRenderer::AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { return outShader->AcquireInstanceResources(rendererState, shader, maps); }
	return INVALID_ID;
}

bool VulkanRenderer::ReleaseInstanceResources(Shader* shader, U32 instanceId)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { return outShader->ReleaseInstanceResources(rendererState, shader, instanceId); }
	return false;
}

bool VulkanRenderer::SetUniform(Shader* shader, Uniform& uniform, const void* value)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { outShader->SetUniform(rendererState, shader, uniform, value); return true; }
	return false;
}

bool VulkanRenderer::SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value)
{
	VulkanShader* outShader = (VulkanShader*)shader->internalData;
	if (outShader) { outShader->SetPushConstant(rendererState, shader, pushConstant, value); return true; }
	return false;
}

bool VulkanRenderer::RecreateSwapchain()
{
	if (rendererState->recreatingSwapchain)
	{
		Logger::Debug("RecreateSwapchain called when already recreating. Booting.");
		return false;
	}

	if (rendererState->framebufferWidth == 0 || rendererState->framebufferHeight == 0)
	{
		Logger::Debug("RecreateSwapchain called when window is < 1 in a dimension. Booting.");
		return false;
	}

	rendererState->recreatingSwapchain = true;

	vkDeviceWaitIdle(rendererState->device->logicalDevice);

	for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
	{
		rendererState->imagesInFlight[i] = 0;
	}

	rendererState->device->QuerySwapchainSupport(rendererState->device->physicalDevice, rendererState->surface, &rendererState->device->swapchainSupport);
	rendererState->device->DetectDepthFormat();

	rendererState->swapchain->Recreate(rendererState, cachedFramebufferWidth, cachedFramebufferHeight);

	rendererState->framebufferSizeLastGeneration = rendererState->framebufferSizeGeneration;

	for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
	{
		rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
	}

	RegenerateRenderTargets();

	CreateCommandBuffers();

	rendererState->recreatingSwapchain = false;

	return true;
}

bool VulkanRenderer::RegenerateRenderTargets()
{
	for (Renderpass* renderpass : Resources::GetRenderpasses())
	{
		for (U8 i = 0; i < renderpass->targets.Size(); ++i)
		{
			if (!DestroyRenderTarget(&renderpass->targets[i], false)) { return false; }

			Texture* windowTargetTexture = GetWindowAttachment(i);

			Vector<Texture*> attachments;
			attachments.Push(windowTargetTexture);

			if (renderpass->clearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG)
			{
				Texture* depthTargetTexture = GetDepthAttachment();
				attachments.Push(depthTargetTexture);
			}

			if (!CreateRenderTarget(attachments, renderpass, rendererState->framebufferWidth, rendererState->framebufferHeight, &renderpass->targets[i])) { return false; }
		}
	}

	return true;
}

bool VulkanRenderer::CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, RenderTarget* target)
{
	Vector<VkImageView> attachmentViews(attachments.Size());
	for (U32 i = 0; i < attachments.Size(); ++i)
	{
		attachmentViews[i] = ((VulkanImage*)attachments[i]->internalData)->view;
	}

	target->attachments = attachments;

	VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = ((VulkanRenderpass*)renderpass->internalData)->handle;
	framebufferInfo.attachmentCount = (U32)attachments.Size();
	framebufferInfo.pAttachments = attachmentViews.Data();
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	VkCheck_FATAL(vkCreateFramebuffer(rendererState->device->logicalDevice, &framebufferInfo, rendererState->allocator, (VkFramebuffer*)&target->internalFramebuffer));

	return true;
}

bool VulkanRenderer::DestroyRenderTarget(RenderTarget* target, bool freeInternalMemory)
{
	if (target && target->internalFramebuffer)
	{
		vkDestroyFramebuffer(rendererState->device->logicalDevice, (VkFramebuffer)target->internalFramebuffer, rendererState->allocator);
		target->internalFramebuffer = nullptr;
		if (freeInternalMemory) { target->attachments.Clear(); }

		return true;
	}

	return false;
}

bool VulkanRenderer::UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, U32& outOffset, U32 size, const void* data)
{
	outOffset = (U32)buffer->Allocate(size);
	if (outOffset == U32_MAX)
	{
		Logger::Error("UploadDataRange: Failed to allocate from the given buffer!");
		return false;
	}

	VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VulkanBuffer staging;
	staging.Create(rendererState, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, false);

	staging.LoadData(rendererState, 0, size, 0, data);

	staging.CopyTo(rendererState, pool, fence, queue, staging.handle, 0, buffer->handle, outOffset, size);

	staging.Destroy(rendererState);

	return true;
}

void VulkanRenderer::FreeDataRange(VulkanBuffer* buffer, U32 offset, U32 size)
{
	if (buffer)
	{
		buffer->Free(size, offset);
	}
}

Texture* VulkanRenderer::GetWindowAttachment(U8 index)
{
	if (index >= rendererState->swapchain->imageCount)
	{
		Logger::Fatal("Attempting to get attachment index out of range: {}. Attachment count: {}", index, rendererState->swapchain->imageCount);
		return nullptr;
	}

	return rendererState->swapchain->renderTextures[index];
}

Texture* VulkanRenderer::GetDepthAttachment()
{
	return rendererState->swapchain->depthTexture;
}

U32 VulkanRenderer::GetWindowAttachmentIndex()
{
	return rendererState->imageIndex;
}