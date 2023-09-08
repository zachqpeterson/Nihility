#pragma once

#include "RenderingDefines.hpp"

struct Rasterization
{
	VkCullModeFlagBits	cullMode{ VK_CULL_MODE_NONE };
	VkFrontFace			front{ VK_FRONT_FACE_COUNTER_CLOCKWISE };
	VkPolygonMode		fill{ VK_POLYGON_MODE_FILL };
};

struct StencilOperationState
{
	VkStencilOp	fail{ VK_STENCIL_OP_KEEP };
	VkStencilOp	pass{ VK_STENCIL_OP_KEEP };
	VkStencilOp	depthFail{ VK_STENCIL_OP_KEEP };
	VkCompareOp	compare{ VK_COMPARE_OP_ALWAYS };
	U32			compareMask{ 0xff };
	U32			writeMask{ 0xff };
	U32			reference{ 0xff };
};

struct DepthStencil
{
	StencilOperationState	front{};
	StencilOperationState	back{};
	VkCompareOp				depthComparison{ VK_COMPARE_OP_ALWAYS };

	bool					depthEnable{ false };
	bool					depthWriteEnable{ false };
	bool					stencilEnable{ false };
};

struct ShaderStage
{
	String							entryPoint{ NO_INIT };
	VkShaderStageFlagBits			stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };

	U32								localSizeX{ 1 };
	U32								localSizeY{ 1 };
	U32								localSizeZ{ 1 };

	bool							usePushConstants{ false };
};

struct Shader
{
	bool Create(const String& shaderPath, U8 pushConstantCount, VkPushConstantRange* pushConstants);
	void Destroy();

	String								name{ NO_INIT };
	U64									handle{ U64_MAX };

	VkPipelineBindPoint					bindPoint{ VK_PIPELINE_BIND_POINT_MAX_ENUM };
	VkPipelineLayout					pipelineLayout{ nullptr };
	VkShaderStageFlags					pushConstantStages{};
	ShaderStage							stages[MAX_SHADER_STAGES]{};
	VkPipelineShaderStageCreateInfo		stageInfos[MAX_SHADER_STAGES]{};
	U32									stageCount{ 0 };
	U32									language{ 0 };

	Rasterization						rasterization{};
	DepthStencil						depthStencil{};
	VkPipelineColorBlendAttachmentState blendStates[MAX_IMAGE_OUTPUTS]{};
	U8									blendStateCount{ 0 };

	DescriptorSetLayout*				setLayouts[MAX_DESCRIPTOR_SETS]{};
	U32									setCount{ 0 };

	U32									vertexStreamCount{ 0 };
	U32									vertexAttributeCount{ 0 };
	VkVertexInputBindingDescription		vertexStreams[MAX_VERTEX_STREAMS]{};
	VkVertexInputAttributeDescription	vertexAttributes[MAX_VERTEX_ATTRIBUTES]{};
	U32									vertexSize{ 0 };

	VkFormat							outputs[MAX_IMAGE_OUTPUTS]{};
	U32									outputCount{ 0 };

	bool								useBindless{ false };
	U8									instanceOffset{ U8_MAX };

private:
	bool ParseConfig(const String& data, I64& index);
	bool ParseStage(const String& data, I64& index, DescriptorSetLayoutInfo* setLayoutInfos, VkShaderStageFlagBits stage);
	VkPipelineShaderStageCreateInfo CompileShader(ShaderStage& shaderStage, String& code, const String& name, DescriptorSetLayoutInfo* setLayoutInfos);
	bool ParseSPIRV(U32* code, U64 codeSize, ShaderStage& stage, DescriptorSetLayoutInfo* setLayoutInfos);

	static const String& ToStageDefines(VkShaderStageFlagBits value);
	static const String& ToCompilerExtension(VkShaderStageFlagBits value);
};