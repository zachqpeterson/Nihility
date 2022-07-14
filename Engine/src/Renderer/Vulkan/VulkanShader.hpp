#pragma once

#include "VulkanDefines.hpp"
#include "Renderer/Renderer.hpp"
#include "Resources/Shader.hpp"

#include "Containers/String.hpp"
#include <Containers/Vector.hpp>

struct ShaderStage
{
	VkShaderModuleCreateInfo info;
	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo shaderStageInfo;
};

struct ShaderStageConfig
{
	VkShaderStageFlagBits stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
	String fileName;
};

struct InstanceState
{
	U32 id;
	U32 offset;

	Vector<VkDescriptorSet> descriptorSets;

	Vector<struct TextureMap> instanceTextureMaps;
};

struct VulkanShaderConfig
{
	Vector<ShaderStageConfig> stages;
	Vector<VkDescriptorPoolSize> poolSizes;
	Array<Vector<VkDescriptorSetLayoutBinding>, 2> descriptorSets;

	Vector<VkVertexInputAttributeDescription> attributes;
};

class VulkanShader
{
public:
	bool Create(RendererState* rendererState, Shader* shader);
	void Destroy(RendererState* rendererState);
	bool Initialize(RendererState* rendererState, Shader* shader);
	bool CreateShaderModule(RendererState* rendererState, ShaderStageConfig config, ShaderStage* shaderStage);

	bool Use(RendererState* rendererState);
	void SetUniform(RendererState* rendererState, Shader* shader, Uniform& uniform, const void* value);
	void SetPushConstant(RendererState* rendererState, Shader* shader, PushConstant& pushConstant, const void* value);

	bool ApplyGlobals(RendererState* rendererState, Shader* shader);

	bool ApplyInstance(RendererState* rendererState, Shader* shader, bool needsUpdate);
	U32 AcquireInstanceResources(RendererState* rendererState, Shader* shader, Vector<TextureMap>& maps);
	bool ReleaseInstanceResources(RendererState* rendererState, Shader* shader, U32 instanceId);

public:
	void* mappedUniformBufferBlock;

	VulkanShaderConfig config;

	Vector<ShaderStage> stages;

	VkDescriptorPool descriptorPool;

	U32 instanceDescriptorUboCount;
	U32 globalDescriptorUboCount;
	Vector<VkDescriptorSetLayout> descriptorSetLayouts;
	Vector<VkDescriptorSet> globalDescriptorSets;

	class VulkanPipeline* pipeline;
	class VulkanBuffer* uniformBuffer;

	Array<InstanceState, VULKAN_MAX_MATERIAL_COUNT> instanceStates;
};