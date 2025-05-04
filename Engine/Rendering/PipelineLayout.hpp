#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"
#include "DescriptorSet.hpp"

#include "Containers/Vector.hpp"

struct NH_API PipelineLayout
{
	bool Create(const Vector<DescriptorSet>& descriptorSets, const Vector<VkPushConstantRange>& pushConstants = {});
	void Destroy();

	operator VkPipelineLayout() const;
	
private:
	VkPipelineLayout vkPipelineLayout;

	friend class Renderer;
	friend struct Pipeline;
};