#pragma once

#include "RenderingDefines.hpp"

enum DrawType
{
	DRAW_TYPE_INDEX,
	DRAW_TYPE_VERTEX,
	DRAW_TYPE_FULLSCREEN,
};

enum PushConstantType
{
	PUSH_CONSTANT_TYPE_NONE,
	PUSH_CONSTANT_TYPE_CAMERA,
	PUSH_CONSTANT_TYPE_POST_PROCESS,
};

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
	String							entryPoint{};
	VkShaderStageFlagBits			stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };

	U32								localSizeX{ 1 };
	U32								localSizeY{ 1 };
	U32								localSizeZ{ 1 };

	bool							usePushConstants{ false };
};

struct Shader
{
	bool Create(const String& shaderPath, U8 pushConstantCount, VkPushConstantRange* pushConstants);
	void AddDescriptor(const Descriptor& descriptor);
	void Destroy();

	String								name{};
	U64									handle{ U64_MAX };

	VkPipelineBindPoint					bindPoint{ VK_PIPELINE_BIND_POINT_MAX_ENUM };
	VkPipelineLayout					pipelineLayout{ nullptr };
	ShaderStage							stages[MAX_SHADER_STAGES]{};
	VkPipelineShaderStageCreateInfo		stageInfos[MAX_SHADER_STAGES]{};
	U32									stageCount{ 0 };
	U32									language{ 0 };
	VkShaderStageFlags					pushConstantStages{};
	PushConstantType					pushConstantType{ PUSH_CONSTANT_TYPE_NONE };
	DrawType							drawType{ DRAW_TYPE_INDEX };

	U8									descriptorCount{ 0 };
	Descriptor							descriptors[MAX_DESCRIPTORS_PER_SET];

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
	U32									outputCount{ 0 };

	bool								useBindless{ false };
	U8									instanceLocation{ U8_MAX };

	U32									renderOrder{ 0 };

private:
	bool ParseConfig(const String& data, I64& index);
	bool ParseStage(const String& data, I64& index, DescriptorSetLayoutInfo& setLayoutInfo, VkShaderStageFlagBits stage);
	VkPipelineShaderStageCreateInfo CompileShader(ShaderStage& shaderStage, String& code, const String& name, DescriptorSetLayoutInfo& setLayoutInfo);
	bool ParseSPIRV(U32* code, U64 codeSize, ShaderStage& stage, DescriptorSetLayoutInfo& setLayoutInfo);

	static const String& ToStageDefines(VkShaderStageFlagBits value);
	static const String& ToCompilerExtension(VkShaderStageFlagBits value);
};