#include "Renderpass.hpp"

#include "Renderer.hpp"

bool Renderpass::Create()
{
	VkAttachmentDescription2 colorAttachment = { VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 };
	colorAttachment.pNext = nullptr;
	colorAttachment.flags = 0;
	colorAttachment.format = Renderer::swapchain.surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference2 colorAttachmentReference = { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 };
	colorAttachmentReference.pNext = nullptr;
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkAttachmentDescription2 depthAttachment = { VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 };
	depthAttachment.pNext = nullptr;
	depthAttachment.flags = 0;
	depthAttachment.flags = 0;
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference2 depthAttachmentReference = { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 };
	depthAttachmentReference.pNext = nullptr;
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachmentReference.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkMemoryBarrier2 depthBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR };
	depthBarrier.pNext = nullptr;
	depthBarrier.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR;
	depthBarrier.srcAccessMask = 0;
	depthBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR;
	depthBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkMemoryBarrier2 renderBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR };
	renderBarrier.pNext = nullptr;
	renderBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	renderBarrier.srcAccessMask = 0;
	renderBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	renderBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	
	VkSubpassDescription2 subpass = { VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2 };
	subpass.pNext = nullptr;
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.viewMask = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkSubpassDependency2 depthDependancy = { VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 };
	depthDependancy.pNext = &depthBarrier;
	depthDependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependancy.dstSubpass = 0;
	depthDependancy.dependencyFlags = 0;
	depthDependancy.viewOffset = 0;

	VkSubpassDependency2 renderDependancy = { VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 };
	renderDependancy.pNext = &renderBarrier;
	renderDependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
	renderDependancy.dstSubpass = 0;
	renderDependancy.dependencyFlags = 0;
	renderDependancy.viewOffset = 0;

	VkSubpassDependency2 dependencies[] = { depthDependancy, renderDependancy };
	VkAttachmentDescription2 attachments[] = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo2 renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = CountOf32(attachments);
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = CountOf32(dependencies);
	renderPassInfo.pDependencies = dependencies;
	renderPassInfo.correlatedViewMaskCount = 0;
	renderPassInfo.pCorrelatedViewMasks = nullptr;

	VkValidateFR(vkCreateRenderPass2(Renderer::device, &renderPassInfo, Renderer::allocationCallbacks, &vkRenderpass));

	return true;
}

void Renderpass::Destroy()
{
	if (vkRenderpass)
	{
		vkDestroyRenderPass(Renderer::device, vkRenderpass, Renderer::allocationCallbacks);
	}
}

Renderpass::operator VkRenderPass() const
{
	return vkRenderpass;
}