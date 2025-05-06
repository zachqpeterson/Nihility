#include "FrameBuffer.hpp"

#include "Renderer.hpp"

bool FrameBuffer::Create()
{
	vkImages = Renderer::swapchain.GetImages();
	vkImageViews = Renderer::swapchain.GetImageViews();

	vkFramebuffers.Resize(vkImages.Size());

    for (U64 i = 0; i < vkFramebuffers.Size(); ++i)
    {
        Vector<VkImageView> attachments = { vkImageViews[i], Renderer::depthTextures[i].imageView };

        VkFramebufferCreateInfo frameBufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        frameBufferInfo.pNext = nullptr;
        frameBufferInfo.flags = 0;
        frameBufferInfo.renderPass = Renderer::renderpass;
        frameBufferInfo.attachmentCount = (U32)attachments.Size();
        frameBufferInfo.pAttachments = attachments.Data();
        frameBufferInfo.width = Renderer::swapchain.extent.width;
        frameBufferInfo.height = Renderer::swapchain.extent.height;
        frameBufferInfo.layers = 1;

        VkValidateFR(vkCreateFramebuffer(Renderer::device, &frameBufferInfo, Renderer::allocationCallbacks, &vkFramebuffers[i]));
    }

    return true;
}

void FrameBuffer::Destroy()
{
    for (VkFramebuffer frameBuffer : vkFramebuffers)
    {
        vkDestroyFramebuffer(Renderer::device, frameBuffer, Renderer::allocationCallbacks);
    }

    for (VkImageView imageView : vkImageViews)
    {
        vkDestroyImageView(Renderer::device, imageView, Renderer::allocationCallbacks);
    }
}