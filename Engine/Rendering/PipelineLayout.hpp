#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Containers/Vector.hpp"

struct PipelineLayout
{
private:
	bool Create(const Vector<VkDescriptorSetLayout>& layouts, const Vector<VkPushConstantRange>& pushConstants = {});
	void Destroy();

	operator VkPipelineLayout() const;

	VkPipelineLayout vkPipelineLayout;

	friend class Renderer;
	friend struct Pipeline;
};