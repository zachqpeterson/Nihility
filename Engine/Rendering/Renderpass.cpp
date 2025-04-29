#include "Renderpass.hpp"

#include "Renderer.hpp"

bool Renderpass::Create()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.flags = 0;
	colorAttachment.format = Renderer::swapchain.surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.flags = 0;
	depthAttachment.format = Renderer::depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference{};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depthDependency{};
	depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependency.dstSubpass = 0;
	depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.srcAccessMask = 0;
	depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	Vector<VkSubpassDependency> dependencies = { subpassDependency, depthDependency };
	Vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = (U32)attachments.Size();
	renderPassInfo.pAttachments = attachments.Data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = (U32)dependencies.Size();
	renderPassInfo.pDependencies = dependencies.Data();

	VkValidateFR(vkCreateRenderPass(Renderer::device, &renderPassInfo, Renderer::allocationCallbacks, &vkRenderpass));

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