#include "VulkanDevice.hpp"

#include "Core/Logger.hpp"
#include "Containers/Vector.hpp"
#include "Memory/Memory.hpp"

//TODO: temporary
#include <string>

struct PhysicalDeviceRequirements
{
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    bool samplerAnisotropy;
    bool discreteGpu;
    Vector<const char*> deviceExtensionNames;
};

struct PhysicalDeviceQueueFamilyInfo
{
    U32 graphicsFamilyIndex;
    U32 presentFamilyIndex;
    U32 computeFamilyIndex;
    U32 transferFamilyIndex;
};

bool VulkanDevice::Create(RendererState* rendererState)
{
    if (!SelectPhysicalDevice(rendererState)) { return false; }

    LOG_INFO("Creating logical device...");

    bool presentSharesGraphicsQueue = rendererState->device->graphicsQueueIndex == rendererState->device->presentQueueIndex;
    bool transferSharesGraphicsQueue = rendererState->device->graphicsQueueIndex == rendererState->device->transferQueueIndex;
    U32 indexCount = 1 + !presentSharesGraphicsQueue + !transferSharesGraphicsQueue;

    U32 indices[32];
    U8 index = 0;
    indices[index] = rendererState->device->graphicsQueueIndex;
    if (!presentSharesGraphicsQueue) { indices[++index] = rendererState->device->presentQueueIndex; }
    if (!transferSharesGraphicsQueue) { indices[++index] = rendererState->device->transferQueueIndex; }

    VkDeviceQueueCreateInfo queueCreateInfos[32];
    for (U32 i = 0; i < indexCount; ++i)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;

        // TODO: Enable this for a future enhancement.
        //if (indices[i] == rendererState->device->graphicsQueueIndex) {
        //    queueCreateInfos[i].queueCount = 2;
        //}
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = 0;
        F32 queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    // TODO: should be config driven
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    bool portabilityRequired = false;
    U32 availableExtensionCount = 0;
    VkExtensionProperties* availableExtensions = 0;
    VkCheck(vkEnumerateDeviceExtensionProperties(rendererState->device->physicalDevice, 0, &availableExtensionCount, 0));

    if (availableExtensionCount != 0)
    {
        availableExtensions = (VkExtensionProperties*)Memory::Allocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        VkCheck(vkEnumerateDeviceExtensionProperties(rendererState->device->physicalDevice, 0, &availableExtensionCount, availableExtensions));
        for (U32 i = 0; i < availableExtensionCount; ++i)
        {
            if (strcmp(availableExtensions[i].extensionName, "VK_KHR_portability_subset") == 0)
            {
                LOG_TRACE("Adding required extension 'VK_KHR_portability_subset'.");
                portabilityRequired = true;
                break;
            }
        }
    }

    Memory::Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);

    Vector<const char*> extensionNames;
    extensionNames.Push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (portabilityRequired) { extensionNames.Push("VK_KHR_portability_subset"); }

    VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.queueCreateInfoCount = indexCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = extensionNames.Size();
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames.Data();
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.pNext = nullptr;

    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;

    VkCheck(vkCreateDevice(rendererState->device->physicalDevice, &deviceCreateInfo, rendererState->allocator, &rendererState->device->logicalDevice));

    LOG_INFO("Obtaining queues...");
    vkGetDeviceQueue(rendererState->device->logicalDevice, rendererState->device->graphicsQueueIndex, 0, &rendererState->device->graphicsQueue);

    vkGetDeviceQueue(rendererState->device->logicalDevice, rendererState->device->presentQueueIndex, 0, &rendererState->device->presentQueue);

    vkGetDeviceQueue(rendererState->device->logicalDevice, rendererState->device->transferQueueIndex, 0, &rendererState->device->transferQueue);

    LOG_INFO("Creating graphics command pool...");
    VkCommandPoolCreateInfo poolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolCreateInfo.queueFamilyIndex = rendererState->device->graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCheck(vkCreateCommandPool(rendererState->device->logicalDevice, &poolCreateInfo, rendererState->allocator, &rendererState->device->graphicsCommandPool));

    return true;
}

void VulkanDevice::Destroy(RendererState* rendererState)
{
    rendererState->device->graphicsQueue = VK_NULL_HANDLE;
    rendererState->device->presentQueue = VK_NULL_HANDLE;
    rendererState->device->transferQueue = VK_NULL_HANDLE;

    LOG_INFO("Destroying vulkan command pools...");
    vkDestroyCommandPool(rendererState->device->logicalDevice, rendererState->device->graphicsCommandPool, rendererState->allocator);

    LOG_INFO("Destroying vulkan logical device...");
    if (rendererState->device->logicalDevice)
    {
        vkDestroyDevice(rendererState->device->logicalDevice, rendererState->allocator);
        rendererState->device->logicalDevice = VK_NULL_HANDLE;
    }

    LOG_INFO("Releasing physical device resources...");
    rendererState->device->physicalDevice = VK_NULL_HANDLE;

    if (rendererState->device->swapchainSupport.formats)
    {
        Memory::Free(rendererState->device->swapchainSupport.formats,
            sizeof(VkSurfaceFormatKHR) * rendererState->device->swapchainSupport.formatCount, MEMORY_TAG_RENDERER);
        rendererState->device->swapchainSupport.formats = VK_NULL_HANDLE;
        rendererState->device->swapchainSupport.formatCount = 0;
    }

    if (rendererState->device->swapchainSupport.presentModes)
    {
        Memory::Free(rendererState->device->swapchainSupport.presentModes,
            sizeof(VkPresentModeKHR) * rendererState->device->swapchainSupport.presentModeCount, MEMORY_TAG_RENDERER);
        rendererState->device->swapchainSupport.presentModes = VK_NULL_HANDLE;
        rendererState->device->swapchainSupport.presentModeCount = 0;
    }

    Memory::Zero(&rendererState->device->swapchainSupport.capabilities, sizeof(rendererState->device->swapchainSupport.capabilities));

    rendererState->device->graphicsQueueIndex = -1;
    rendererState->device->presentQueueIndex = -1;
    rendererState->device->transferQueueIndex = -1;
}

bool VulkanDevice::SelectPhysicalDevice(RendererState* rendererState)
{
    LOG_INFO("Selecting physical device...");

    U32 physicalDeviceCount = 0;
    VkCheck(vkEnumeratePhysicalDevices(rendererState->instance, &physicalDeviceCount, 0));
    if (physicalDeviceCount == 0)
    {
        LOG_FATAL("No devices which support Vulkan were found.");
        return false;
    }

    VkPhysicalDevice physicalDevices[32];
    VkCheck(vkEnumeratePhysicalDevices(rendererState->instance, &physicalDeviceCount, physicalDevices));
    for (U32 i = 0; i < physicalDeviceCount; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

        LOG_TRACE("Evaluating device: '%s', index %u.", properties.deviceName, i);

        bool supportsDeviceLocalHostVisible = false;
        for (U32 i = 0; i < memory.memoryTypeCount; ++i)
        {
            if (((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
                ((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0))
            {
                supportsDeviceLocalHostVisible = true;
                break;
            }
        }

        // TODO: These requirements should probably be driven by engine
        // configuration.
        PhysicalDeviceRequirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = true;
        requirements.samplerAnisotropy = true;
#if KPLATFORM_APPLE
        requirements.discreteGpu = false;
#else
        requirements.discreteGpu = true;
#endif
        requirements.deviceExtensionNames.Push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        PhysicalDeviceQueueFamilyInfo queueInfo = {};
        bool result = physicalDeviceMeetsRequirements(physicalDevices[i], rendererState->surface, &properties,
            &features, &requirements, &queueInfo, &rendererState->device->swapchainSupport);

        if (result)
        {
#if LOG_TRACE_ENABLED == 1
            LOG_TRACE("Selected device: '%s'.", properties.deviceName);
            switch (properties.deviceType)
            {
            default:
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                LOG_TRACE("GPU type is Unknown.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                LOG_TRACE("GPU type is Integrated.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                LOG_TRACE("GPU type is Descrete.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                LOG_TRACE("GPU type is Virtual.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                LOG_TRACE("GPU type is CPU.");
                break;
            }

            LOG_TRACE("GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            LOG_TRACE("Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            for (U32 j = 0; j < memory.memoryHeapCount; ++j)
            {
                F32 memorySizeGb = (((F32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    LOG_TRACE("Local GPU memory: %.2f GB", memorySizeGb);
                }
                else
                {
                    LOG_TRACE("Shared System memory: %.2f GB", memorySizeGb);
                }
            }
#endif

            rendererState->device->physicalDevice = physicalDevices[i];
            rendererState->device->graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            rendererState->device->presentQueueIndex = queueInfo.presentFamilyIndex;
            rendererState->device->transferQueueIndex = queueInfo.transferFamilyIndex;
            // TODO: set compute index here if needed.

            rendererState->device->properties = properties;
            rendererState->device->features = features;
            rendererState->device->memory = memory;
            rendererState->device->supportsDeviceLocalHostVisible = supportsDeviceLocalHostVisible;
            break;
        }
    }

    if (!rendererState->device->physicalDevice)
    {
        LOG_ERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    return true;
}

bool VulkanDevice::physicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const PhysicalDeviceRequirements* requirements,
    PhysicalDeviceQueueFamilyInfo* outQueueInfo,
    SwapchainSupportInfo* outSwapchainSupport)
{
    outQueueInfo->graphicsFamilyIndex = -1;
    outQueueInfo->presentFamilyIndex = -1;
    outQueueInfo->computeFamilyIndex = -1;
    outQueueInfo->transferFamilyIndex = -1;

    if (requirements->discreteGpu)
    {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            LOG_TRACE("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    U32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
    VkQueueFamilyProperties queueFamilies[32];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    U8 minTransferScore = 255;
    for (U32 i = 0; i < queueFamilyCount; ++i)
    {
        U8 currentTransferScore = 0;

        if (outQueueInfo->graphicsFamilyIndex == -1 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            outQueueInfo->graphicsFamilyIndex = i;
            ++currentTransferScore;

            VkBool32 supportsPresent = VK_FALSE;
            VkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent));
            if (supportsPresent)
            {
                outQueueInfo->presentFamilyIndex = i;
                ++currentTransferScore;
            }
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            outQueueInfo->computeFamilyIndex = i;
            ++currentTransferScore;
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if (currentTransferScore <= minTransferScore)
            {
                minTransferScore = currentTransferScore;
                outQueueInfo->transferFamilyIndex = i;
            }
        }
    }

    if (outQueueInfo->presentFamilyIndex == -1)
    {
        for (U32 i = 0; i < queueFamilyCount; ++i)
        {
            VkBool32 supportsPresent = VK_FALSE;
            VkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent));
            if (supportsPresent)
            {
                outQueueInfo->presentFamilyIndex = i;

                if (outQueueInfo->presentFamilyIndex != outQueueInfo->graphicsFamilyIndex)
                {
                    LOG_WARN("Warning: Different queue index used for present vs graphics: %u.", i);
                }
                break;
            }
        }
    }

    LOG_TRACE("Graphics | Present | Compute | Transfer | Name");
    LOG_TRACE("       %d |       %d |       %d |        %d | %s",
        outQueueInfo->graphicsFamilyIndex != -1,
        outQueueInfo->presentFamilyIndex != -1,
        outQueueInfo->computeFamilyIndex != -1,
        outQueueInfo->transferFamilyIndex != -1,
        properties->deviceName);

    if ((!requirements->graphics || (requirements->graphics && outQueueInfo->graphicsFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && outQueueInfo->presentFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && outQueueInfo->computeFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && outQueueInfo->transferFamilyIndex != -1)))
    {
        LOG_TRACE("Device meets queue requirements.");
        LOG_TRACE("Graphics Family Index: %i", outQueueInfo->graphicsFamilyIndex);
        LOG_TRACE("Present Family Index:  %i", outQueueInfo->presentFamilyIndex);
        LOG_TRACE("Transfer Family Index: %i", outQueueInfo->transferFamilyIndex);
        LOG_TRACE("Compute Family Index:  %i", outQueueInfo->computeFamilyIndex);

        QuerySwapchainSupport(device, surface, outSwapchainSupport);

        if (outSwapchainSupport->formatCount < 1 || outSwapchainSupport->presentModeCount < 1)
        {
            if (outSwapchainSupport->formats)
            {
                Memory::Free(outSwapchainSupport->formats, sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
            }
            if (outSwapchainSupport->presentModes)
            {
                Memory::Free(outSwapchainSupport->presentModes, sizeof(VkPresentModeKHR) * outSwapchainSupport->presentModeCount, MEMORY_TAG_RENDERER);
            }
            LOG_TRACE("Required swapchain support not present, skipping device->");
            return false;
        }

        if (requirements->deviceExtensionNames.Size())
        {
            U32 availableExtensionCount = 0;
            VkExtensionProperties* availableExtensions = 0;
            VkCheck(vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, 0));

            if (availableExtensionCount != 0)
            {
                availableExtensions = (VkExtensionProperties*)Memory::Allocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                VkCheck(vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, availableExtensions));

                U32 requiredExtensionCount = requirements->deviceExtensionNames.Size();
                for (U32 i = 0; i < requiredExtensionCount; ++i)
                {
                    bool found = false;
                    for (U32 j = 0; j < availableExtensionCount; ++j)
                    {
                        if (strcmp(requirements->deviceExtensionNames[i], availableExtensions[j].extensionName) == 0)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        LOG_TRACE("Required extension not found: '%s', skipping device->", requirements->deviceExtensionNames[i]);
                        Memory::Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            Memory::Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        }

        if (requirements->samplerAnisotropy && !features->samplerAnisotropy)
        {
            LOG_TRACE("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        return true;
    }

    return false;
}

void VulkanDevice::QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainSupportInfo* outSupportInfo)
{
    VkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &outSupportInfo->capabilities));

    VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, 0));

    if (outSupportInfo->formatCount != 0)
    {
        if (!outSupportInfo->formats)
        {
            outSupportInfo->formats = (VkSurfaceFormatKHR*)Memory::Allocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
        }
        VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, outSupportInfo->formats));
    }

    VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &outSupportInfo->presentModeCount, 0));
    if (outSupportInfo->presentModeCount != 0)
    {
        if (!outSupportInfo->presentModes)
        {
            outSupportInfo->presentModes = (VkPresentModeKHR*)Memory::Allocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModeCount, MEMORY_TAG_RENDERER);
        }
        VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &outSupportInfo->presentModeCount, outSupportInfo->presentModes));
    }
}

bool VulkanDevice::DetectDepthFormat(RendererState* rendererState)
{
    const U64 candidate_count = 3;
    VkFormat candidates[3] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

    U32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (U64 i = 0; i < candidate_count; ++i)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(rendererState->device->physicalDevice, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags)
        {
            rendererState->device->depthFormat = candidates[i];
            return true;
        }
        else if ((properties.optimalTilingFeatures & flags) == flags)
        {
            rendererState->device->depthFormat = candidates[i];
            return true;
        }
    }

    return false;
}