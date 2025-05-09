#pragma once

#include "ResourceDefines.hpp"

#include "Rendering/Buffer.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/PipelineLayout.hpp"
#include "Rendering/Pipeline.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"

struct NH_API Material
{
	bool Create(const PipelineLayout& pipelineLayout, const Pipeline& pipeline, const Vector<VkDescriptorSet>& descriptorSets);
	void Destroy();

	void Bind(CommandBuffer commandBuffer, U32 instanceCount) const;

	PipelineLayout pipelineLayout;
	Pipeline pipeline;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	Buffer instanceBuffers[MaxSwapchainImages];
	Vector<VkDescriptorSet> sets;
};