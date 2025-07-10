#include "FrameBuffer.hpp"

#include "Renderer.hpp"

bool FrameBuffer::Create()
{
	vkFramebuffers.Resize(Renderer::swapchain.imageCount);

	for (U64 i = 0; i < vkFramebuffers.Size(); ++i)
	{
		Vector<VkImageView> attachments = { Renderer::colorTextures[i].imageView, Renderer::depthTextures[i].imageView, Renderer::swapchain.imageViews[i] };

		VkFramebufferCreateInfo frameBufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		frameBufferInfo.pNext = nullptr;
		frameBufferInfo.flags = 0;
		frameBufferInfo.renderPass = Renderer::renderpass;
		frameBufferInfo.attachmentCount = (U32)attachments.Size();
		frameBufferInfo.pAttachments = attachments.Data();
		frameBufferInfo.width = Renderer::swapchain.width;
		frameBufferInfo.height = Renderer::swapchain.height;
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
}


FrameBuffer::operator VkFramebuffer_T* () const
{
	return vkFramebuffers[Renderer::frameIndex];
}