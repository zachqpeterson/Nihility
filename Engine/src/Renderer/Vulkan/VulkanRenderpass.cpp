#include "VulkanRenderpass.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanCommandBuffer.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

#undef ZeroMemory
#undef CopyMemory

bool VulkanRenderpass::Create(RendererState* rendererState,
    Vector4 renderArea,
    Vector4 clearColor,
    F32 depth,
    U32 stencil,
    U8 clearFlags,
    bool hasPrevPass,
    bool hasNextPass)
{
    LOG_INFO("Creating vulkan renderpass...");

    this->clearFlags = clearFlags;
    this->renderArea = renderArea;
    this->clearColor = clearColor;
    this->hasPrevPass = hasPrevPass;
    this->hasNextPass = hasNextPass;

    this->depth = depth;
    this->stencil = stencil;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Attachments TODO: make this configurable.
    U32 attachmentDescriptionCount = 0;
    VkAttachmentDescription attachmentDescriptions[2];

    bool doClearColour = (this->clearFlags & RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG) != 0;
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = rendererState->swapchain->imageFormat.format;  // TODO: configurable
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = doClearColour ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // If coming from a previous pass, should already be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. Otherwise undefined.
    colorAttachment.initialLayout = this->hasPrevPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

    // If going to another pass, use VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. Otherwise VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
    colorAttachment.finalLayout = this->hasNextPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachmentDescriptions[attachmentDescriptionCount] = colorAttachment;
    ++attachmentDescriptionCount;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    VkAttachmentReference depthAttachmentReference;
    bool doClearDepth = (this->clearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0;
    if (doClearDepth)
    {
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = rendererState->device->depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = doClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachmentDescriptions[attachmentDescriptionCount] = depthAttachment;
        ++attachmentDescriptionCount;

        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depthAttachmentReference;
    }
    else
    {
        Memory::ZeroMemory(&attachmentDescriptions[attachmentDescriptionCount], sizeof(VkAttachmentDescription));
        subpass.pDepthStencilAttachment = nullptr;
    }

    // TODO: other attachment types (input, resolve, preserve)
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

    VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderPassInfo.attachmentCount = attachmentDescriptionCount;
    renderPassInfo.pAttachments = attachmentDescriptions;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;

    VkCheck(vkCreateRenderPass(
        rendererState->device->logicalDevice,
        &renderPassInfo,
        rendererState->allocator,
        &handle));

    return true;
}

void VulkanRenderpass::Destroy(RendererState* rendererState)
{
    LOG_INFO("Destroying vulkan render pass...");

    if (handle)
    {
        vkDestroyRenderPass(rendererState->device->logicalDevice, handle, rendererState->allocator);
        handle = nullptr;
    }
}

void VulkanRenderpass::Begin(VulkanCommandBuffer* commandBuffer, VkFramebuffer frameBuffer)
{
    VkRenderPassBeginInfo beginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    beginInfo.renderPass = handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = renderArea.x;
    beginInfo.renderArea.offset.y = renderArea.y;
    beginInfo.renderArea.extent.width = renderArea.z;
    beginInfo.renderArea.extent.height = renderArea.w;

    beginInfo.clearValueCount = 0;
    beginInfo.pClearValues = nullptr;

    VkClearValue clearValues[2];
    Memory::ZeroMemory(clearValues, sizeof(VkClearValue) * 2);
    bool doClearColour = (clearFlags & RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG) != 0;
    if (doClearColour)
    {
        Memory::CopyMemory(clearValues[beginInfo.clearValueCount].color.float32, &clearColor, sizeof(F32) * 4);
        ++beginInfo.clearValueCount;
    }

    bool doClearDepth = (clearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0;
    if (doClearDepth)
    {
        Memory::CopyMemory(clearValues[beginInfo.clearValueCount].color.float32, &clearColor, sizeof(F32) * 4);
        clearValues[beginInfo.clearValueCount].depthStencil.depth = depth;

        bool doClearStencil = (clearFlags & RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG) != 0;
        clearValues[beginInfo.clearValueCount].depthStencil.stencil = doClearStencil ? stencil : 0;
        ++beginInfo.clearValueCount;
    }

    beginInfo.pClearValues = beginInfo.clearValueCount > 0 ? clearValues : nullptr;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanRenderpass::End(VulkanCommandBuffer* commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}