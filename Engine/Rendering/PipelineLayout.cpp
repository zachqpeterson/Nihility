#include "PipelineLayout.hpp"

#include "Renderer.hpp"

bool PipelineLayout::Create(const Vector<VkDescriptorSetLayout>& layouts, const Vector<VkPushConstantRange>& pushConstants)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.pNext = nullptr;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = (U32)layouts.Size();
	pipelineLayoutInfo.pSetLayouts = layouts.Data();
	pipelineLayoutInfo.pushConstantRangeCount = (U32)pushConstants.Size();
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.Data();

	VkValidateFR(vkCreatePipelineLayout(Renderer::device, &pipelineLayoutInfo, Renderer::allocationCallbacks, &vkPipelineLayout));

	return true;
}

void PipelineLayout::Destroy()
{
	if (vkPipelineLayout)
	{
		vkDestroyPipelineLayout(Renderer::device, vkPipelineLayout, Renderer::allocationCallbacks);
	}
}

PipelineLayout::operator VkPipelineLayout() const
{
	return vkPipelineLayout;
}