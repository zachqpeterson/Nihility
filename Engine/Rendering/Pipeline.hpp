#pragma once

#include "RenderingDefines.hpp"
#include "Shader.hpp"

struct DescriptorSetLayoutCreation;
struct DescriptorSetLayout;
struct DescriptorSet;
struct Renderpass;
struct Buffer;

struct SpecializationInfo
{
	VkSpecializationInfo		specializationInfo{};
	U8*							specializationBuffer[MAX_SPECIALIZATION_CONSTANTS * sizeof(U32)];
};

enum ConnectionType
{
	CONNECTION_TYPE_RENDERTARGET,
	CONNECTION_TYPE_DEPTHBUFFER,
	CONNECTION_TYPE_BUFFER,
};

struct Pipeline;

struct PipelineConnection
{
	ConnectionType type;

	Pipeline* pipeline{ nullptr };
	Buffer* buffer{ nullptr };

	U32 index;
	U32 set;
	U32 binding;
};

struct RenderpassCreation;

struct Pipeline
{
	bool Create(Shader* shader, Renderpass* renderpass);
	void Destroy();

	void Resize();

	void AddConnection(const PipelineConnection& connection);
	void UpdateDescriptors();

	String				name{ NO_INIT };
	U64					handle{ U64_MAX };

	Shader*				shader{};
	Renderpass*			renderpass{ nullptr };
	VkPipeline			pipeline{ nullptr };

	DescriptorSet*		descriptorSets[MAX_SWAPCHAIN_IMAGES][MAX_DESCRIPTOR_SETS];
	U8					descriptorSetCount{ 0 };

	PipelineConnection	pipelineConnections[MAX_PIPELINE_CONNECTIONS];
	U8					connectionCount;

private:
	bool CreatePipeline(VkDescriptorSetLayout* vkLayouts);
};