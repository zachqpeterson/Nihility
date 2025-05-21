#pragma once

#include "Defines.hpp"

#include "DescriptorSet.hpp"

#include "Containers/Vector.hpp"

struct VkPushConstantRange;
struct VkPipelineLayout_T;

struct NH_API PipelineLayout
{
	bool Create(const Vector<DescriptorSet>& descriptorSets, const Vector<VkPushConstantRange>& pushConstants = {});
	void Destroy();

	operator VkPipelineLayout_T* () const;

private:
	VkPipelineLayout_T* vkPipelineLayout;

	friend class Renderer;
	friend struct Pipeline;
};