#pragma once

#include "ResourceDefines.hpp"

#include "Rendering/Buffer.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/PipelineLayout.hpp"
#include "Rendering/Pipeline.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"

struct VkDescriptorSet_T;

struct NH_API PushConstant
{
	operator bool() const { return data && size; }

	const void* data;
	U32 size;
	U32 offset;
	U32 stages;
};

enum class VertexUsage
{
	VerticesAndInstances,
	Vertices,
	Instances,
	None
};

struct NH_API Material
{
public:
	bool Create(const PipelineLayout& pipelineLayout, const Pipeline& pipeline, const Vector<VkDescriptorSet_T*>& descriptorSets = {}, const Vector<PushConstant>& pushConstants = {});
	void Destroy();

	void UploadVertices(void* data, U32 size, U32 offset);
	void UploadInstances(void* data, U32 size, U32 offset);
	void UploadIndices(void* data, U32 size, U32 offset);

	const PipelineLayout& GetPipelineLayout() const;
	
	void Bind(CommandBuffer commandBuffer) const;

private:
	PipelineLayout pipelineLayout;
	Pipeline pipeline;
	VertexUsage vertexUsage;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	Buffer instanceBuffers[MaxSwapchainImages];
	Vector<VkDescriptorSet_T*> sets;
	Vector<PushConstant> pushConstants;
};