#include "VulkanRenderer.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanImage.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanBuffer.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"
#include "Math/Math.hpp"

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
        LOG_ERROR(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARN(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_INFO(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_TRACE(callbackData->pMessage); break;
    }

    return VK_FALSE;
}

static RendererState* rendererState;
static U32 cachedFramebufferWidth = 0;
static U32 cachedFramebufferHeight = 0;

bool VulkanRenderer::Initialize()
{
    rendererState = (RendererState*)Memory::Allocate(sizeof(RendererState), MEMORY_TAG_RENDERER);
    rendererState->FindMemoryIndex = FindMemoryIndex;
    rendererState->device = (VulkanDevice*)Memory::Allocate(sizeof(VulkanDevice), MEMORY_TAG_RENDERER);
    rendererState->swapchain = (VulkanSwapchain*)Memory::Allocate(sizeof(VulkanSwapchain), MEMORY_TAG_RENDERER);
    rendererState->mainRenderpass = (VulkanRenderpass*)Memory::Allocate(sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);
    rendererState->uiRenderpass = (VulkanRenderpass*)Memory::Allocate(sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);
    rendererState->objectIndexBuffer = (VulkanBuffer*)Memory::Allocate(sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);
    rendererState->objectVertexBuffer = (VulkanBuffer*)Memory::Allocate(sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);

    rendererState->framebufferWidth = 1280; //TODO: Get width from platform
    rendererState->framebufferHeight = 720;

    rendererState->allocator = nullptr;

    if (!CreateInstance() || !CreateDebugger() || !CreateSurface()) { return false; }

    rendererState->device->Create(rendererState);
    rendererState->swapchain->Create(rendererState, rendererState->framebufferWidth, rendererState->framebufferHeight);

    rendererState->mainRenderpass->Create(rendererState, { 0, 0, (F32)rendererState->framebufferWidth, (F32)rendererState->framebufferHeight }, { 0.0f, 0.0f, 0.2f, 1.0f },
        1.0f, 0, RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG | RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG | RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG, false, true);

    rendererState->uiRenderpass->Create(rendererState, { 0, 0, (F32)rendererState->framebufferWidth, (F32)rendererState->framebufferHeight }, { 0.0f, 0.0f, 0.0f, 0.0f },
        1.0f, 0, RENDERPASS_CLEAR_NONE_FLAG, true, false);

    RecreateFramebuffers();

    CreateCommandBuffers();

    CreateSyncObjects();

    CreateBuffers();

    return true;
}

void VulkanRenderer::Shutdown()
{
    vkDeviceWaitIdle(rendererState->device->logicalDevice);

    LOG_INFO("Destroying vulkan buffers...");
    rendererState->objectIndexBuffer->Destroy(rendererState);
    rendererState->objectVertexBuffer->Destroy(rendererState);
    Memory::Free(rendererState->objectIndexBuffer, sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);
    Memory::Free(rendererState->objectVertexBuffer, sizeof(VulkanBuffer), MEMORY_TAG_RENDERER);

    LOG_INFO("Destroying vulkan sync objects...");

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

    LOG_INFO("Destroying vulkan commandbuffers...");
    for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        if (rendererState->graphicsCommandBuffers[i].handle)
        {
            rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
            rendererState->graphicsCommandBuffers[i].handle = nullptr;
        }
    }

    LOG_INFO("Destroying vulkan framebuffers...");
    for (I32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        vkDestroyFramebuffer(rendererState->device->logicalDevice, rendererState->worldFramebuffers[i], rendererState->allocator);
        vkDestroyFramebuffer(rendererState->device->logicalDevice, rendererState->swapchain->framebuffers[i], rendererState->allocator);
    }

    rendererState->uiRenderpass->Destroy(rendererState);
    rendererState->mainRenderpass->Destroy(rendererState);
    Memory::Free(rendererState->uiRenderpass, sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);
    Memory::Free(rendererState->mainRenderpass, sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);

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

bool VulkanRenderer::CreateInstance()
{
    LOG_INFO("Creating vulkan instance...");

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_VERSION_1_3;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.pApplicationName = "TEST";
    appInfo.pEngineName = "Nihility";
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.pApplicationInfo = &appInfo;

    Vector<const char*> extentionNames;
    extentionNames.Push(VK_KHR_SURFACE_EXTENSION_NAME);
    GetPlatformExtentions(&extentionNames);

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
            LOG_FATAL("Required validation layer is missing: %s", (char*)layerNames[i]);
            return false;
        }
    }

    instanceInfo.enabledLayerCount = layerNames.Size();
    instanceInfo.ppEnabledLayerNames = layerNames.Data();
#else
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
#endif

    instanceInfo.enabledExtensionCount = extentionNames.Size();
    instanceInfo.ppEnabledExtensionNames = extentionNames.Data();

    instanceInfo.flags = NULL;
    instanceInfo.pNext = nullptr;

    return vkCreateInstance(&instanceInfo, rendererState->allocator, &rendererState->instance) == VK_SUCCESS;
}

bool VulkanRenderer::CreateDebugger()
{
#ifdef NH_DEBUG
    LOG_DEBUG("Creating vulkan debugger...");
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
    LOG_INFO("Creating vulkan surface...");

#ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = NULL;
    Platform::GetVulkanSurfaceInfo(&surfaceInfo.flags + sizeof(VkWin32SurfaceCreateFlagsKHR));
    return vkCreateWin32SurfaceKHR(rendererState->instance, &surfaceInfo, rendererState->allocator, &rendererState->surface) == VK_SUCCESS;
#elif PLATFORM_LINUX
    //TODO:
#elif PLATFORM_APPLE
    //TODO:
#endif
    return false;
}

void VulkanRenderer::CreateCommandBuffers()
{
    LOG_INFO("Creating vulkan command buffers...");

    if (!rendererState->graphicsCommandBuffers.Size())
    {
        rendererState->graphicsCommandBuffers.Resize(rendererState->swapchain->imageCount);
        for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
        {
            Memory::Zero(&rendererState->graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }

    for (I32 i = 0; i < rendererState->swapchain->imageCount; ++i)
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

void VulkanRenderer::GetPlatformExtentions(Vector<const char*>* names)
{
#ifdef PLATFORM_WINDOWS
    names->Push("VK_KHR_win32_surface");
#elif PLATFORM_LINUX
    names->Push("VK_KHR_xcb_surface");
#elif PLATFORM_APPLE
    names->Push("VK_EXT_metal_surface");
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

    LOG_ERROR("Unable to find suitable memory type!");
    return -1;
}

bool VulkanRenderer::CreateBuffers()
{
    LOG_INFO("Creating vulkan buffers...");

    VkMemoryPropertyFlagBits memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // Geometry vertex buffer
    const U64 vertexBufferSize = sizeof(Vertex3) * 1024 * 1024;

    if (!rendererState->objectVertexBuffer->Create(rendererState, vertexBufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true, true))
    {
        LOG_FATAL("Error creating vertex buffer.");
        return false;
    }

    // Geometry index buffer
    const U64 indexBufferSize = sizeof(U32) * 1024 * 1024;
    if (!rendererState->objectIndexBuffer->Create(rendererState, indexBufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
        memoryPropertyFlags, true, true))
    {
        LOG_FATAL("Error creating vertex buffer.");
        return false;
    }

    return true;
}

bool VulkanRenderer::BeginFrame()
{
    if (rendererState->recreatingSwapchain)
    {
        VkCheck_ERROR(vkDeviceWaitIdle(rendererState->device->logicalDevice));

        LOG_INFO("Recreating swapchain, booting.");
        return false;
    }

    if (rendererState->framebufferSizeGeneration != rendererState->framebufferSizeLastGeneration)
    {
        VkCheck_ERROR(vkDeviceWaitIdle(rendererState->device->logicalDevice));

        if (!RecreateSwapchain()) { return false; }

        LOG_INFO("Resized, booting.");
        return false;
    }

    VkCheck_FATAL(vkWaitForFences(rendererState->device->logicalDevice, 1, &rendererState->inFlightFences[rendererState->currentFrame], true, UINT64_MAX));

    if (!rendererState->swapchain->AcquireNextImageIndex(rendererState, UINT64_MAX,
        rendererState->imageAvailableSemaphores[rendererState->currentFrame], 0, &rendererState->imageIndex))
    {
        LOG_ERROR("Failed to acquire next image index, booting.");
        return false;
    }

    VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];
    commandBuffer->Reset();
    commandBuffer->Begin(false, false, false);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (F32)rendererState->framebufferHeight;
    viewport.width = (F32)rendererState->framebufferWidth;
    viewport.height = -(F32)rendererState->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = rendererState->framebufferWidth;
    scissor.extent.height = rendererState->framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    rendererState->mainRenderpass->renderArea.z = rendererState->framebufferWidth;
    rendererState->mainRenderpass->renderArea.w = rendererState->framebufferHeight;

    rendererState->uiRenderpass->renderArea.z = rendererState->framebufferWidth;
    rendererState->uiRenderpass->renderArea.w = rendererState->framebufferHeight;

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

bool VulkanRenderer::BeginRenderpass(U8 renderpassId)
{
    VulkanRenderpass* renderpass = nullptr;
    VkFramebuffer framebuffer = 0;
    VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];

    //TODO: Find a better way to do this
    switch (renderpassId)
    {
    case 0:
        renderpass = rendererState->mainRenderpass;
        framebuffer = rendererState->worldFramebuffers[rendererState->imageIndex];
        break;
    case 1:
        renderpass = rendererState->uiRenderpass;
        framebuffer = rendererState->swapchain->framebuffers[rendererState->imageIndex];
        break;
    default:
        LOG_ERROR("vulkan_renderer_begin_renderpass called on unrecognized renderpass id: %#02x", renderpassId);
        return false;
    }

    renderpass->Begin(commandBuffer, framebuffer);

    return true;
}

bool VulkanRenderer::EndRenderpass(U8 renderpassId)
{
    VulkanRenderpass* renderpass = nullptr;
    VulkanCommandBuffer* commandBuffer = &rendererState->graphicsCommandBuffers[rendererState->imageIndex];

    switch (renderpassId)
    {
    case 0:
        renderpass = rendererState->mainRenderpass;
        break;
    case 1:
        renderpass = rendererState->uiRenderpass;
        break;
    default:
        LOG_ERROR("vulkan_renderer_end_renderpass called on unrecognized renderpass id:  %#02x", renderpassId);
        return false;
    }

    renderpass->End(commandBuffer);
    return true;
}

void VulkanRenderer::DrawMesh() //TODO: Pass info
{
    //if (data.geometry && data.geometry->internal_id == INVALID_ID)
    //{
    //    return;
    //}
    //
    //vulkan_geometry_data* bufferData = &renderpass->geometries[data.geometry->internal_id];
    //VulkanCommandBuffer* commandBuffer = &renderpass->graphicsCommandBuffers[renderpass->imageIndex];
    //
    //VkDeviceSize offsets[1] = { bufferData->vertex_buffer_offset };
    //vkCmdBindVertexBuffers(commandBuffer->handle, 0, 1, &renderpass->object_vertex_buffer.handle, (VkDeviceSize*)offsets);
    //
    //if (bufferData->index_count > 0)
    //{
    //    vkCmdBindIndexBuffer(commandBuffer->handle, renderpass->object_index_buffer.handle, bufferData->index_buffer_offset, VK_INDEX_TYPE_UINT32);
    //
    //    vkCmdDrawIndexed(commandBuffer->handle, bufferData->index_count, 1, 0, 0, 0);
    //}
    //else
    //{
    //    vkCmdDraw(commandBuffer->handle, bufferData->vertex_count, 1, 0, 0);
    //}
}

bool VulkanRenderer::RecreateSwapchain()
{
    if (rendererState->recreatingSwapchain)
    {
        LOG_DEBUG("RecreateSwapchain called when already recreating. Booting.");
        return false;
    }

    if (rendererState->framebufferWidth == 0 || rendererState->framebufferHeight == 0)
    {
        LOG_DEBUG("RecreateSwapchain called when window is < 1 in a dimension. Booting.");
        return false;
    }

    rendererState->recreatingSwapchain = true;

    vkDeviceWaitIdle(rendererState->device->logicalDevice);

    for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        rendererState->imagesInFlight[i] = 0;
    }

    rendererState->device->QuerySwapchainSupport(rendererState->device->physicalDevice, rendererState->surface, &rendererState->device->swapchainSupport);
    rendererState->device->DetectDepthFormat(rendererState);

    rendererState->swapchain->Recreate(rendererState, cachedFramebufferWidth, cachedFramebufferHeight);

    rendererState->framebufferWidth = cachedFramebufferWidth;
    rendererState->framebufferHeight = cachedFramebufferHeight;
    rendererState->mainRenderpass->renderArea.z = rendererState->framebufferWidth;
    rendererState->mainRenderpass->renderArea.w = rendererState->framebufferHeight;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    rendererState->framebufferSizeLastGeneration = rendererState->framebufferSizeGeneration;

    for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
    }

    for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        vkDestroyFramebuffer(rendererState->device->logicalDevice, rendererState->worldFramebuffers[i], rendererState->allocator);
        vkDestroyFramebuffer(rendererState->device->logicalDevice, rendererState->swapchain->framebuffers[i], rendererState->allocator);
    }

    rendererState->mainRenderpass->renderArea.x = 0;
    rendererState->mainRenderpass->renderArea.y = 0;
    rendererState->mainRenderpass->renderArea.z = rendererState->framebufferWidth;
    rendererState->mainRenderpass->renderArea.w = rendererState->framebufferHeight;

    rendererState->uiRenderpass->renderArea.x = 0;
    rendererState->uiRenderpass->renderArea.y = 0;
    rendererState->uiRenderpass->renderArea.z = rendererState->framebufferWidth;
    rendererState->uiRenderpass->renderArea.w = rendererState->framebufferHeight;

    RecreateFramebuffers();
    CreateCommandBuffers();

    rendererState->recreatingSwapchain = false;

    return true;
}

void VulkanRenderer::RecreateFramebuffers()
{
    LOG_INFO("Creating vulkan framebuffers...");

    U32 imageCount = rendererState->swapchain->imageCount;
    for (U32 i = 0; i < imageCount; ++i)
    {
        VkImageView worldAttachments[2] = { rendererState->swapchain->views[i], rendererState->swapchain->depthAttachment->view };
        VkFramebufferCreateInfo worldFramebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        worldFramebufferInfo.renderPass = rendererState->mainRenderpass->handle;
        worldFramebufferInfo.attachmentCount = 2;
        worldFramebufferInfo.pAttachments = worldAttachments;
        worldFramebufferInfo.width = rendererState->framebufferWidth;
        worldFramebufferInfo.height = rendererState->framebufferHeight;
        worldFramebufferInfo.layers = 1;

        VkCheck(vkCreateFramebuffer(rendererState->device->logicalDevice, &worldFramebufferInfo, rendererState->allocator, &rendererState->worldFramebuffers[i]));

        // Swapchain framebuffers (UI pass). Outputs to swapchain images
        VkImageView uiAttachments[1] = { rendererState->swapchain->views[i] };
        VkFramebufferCreateInfo uiFramebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        uiFramebufferInfo.renderPass = rendererState->uiRenderpass->handle;
        uiFramebufferInfo.attachmentCount = 1;
        uiFramebufferInfo.pAttachments = uiAttachments;
        uiFramebufferInfo.width = rendererState->framebufferWidth;
        uiFramebufferInfo.height = rendererState->framebufferHeight;
        uiFramebufferInfo.layers = 1;

        VkCheck(vkCreateFramebuffer(rendererState->device->logicalDevice, &uiFramebufferInfo, rendererState->allocator, &rendererState->swapchain->framebuffers[i]));
    }
}