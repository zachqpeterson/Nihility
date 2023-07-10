#pragma once

#include "RenderingDefines.hpp"

struct DescriptorSetLayoutCreation;
struct DescriptorSetLayout;
struct DescriptorSet;
struct Renderpass;
struct Buffer;

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

struct BlendState
{
	VkBlendFactor			sourceColor{ VK_BLEND_FACTOR_ONE };
	VkBlendFactor			destinationColor{ VK_BLEND_FACTOR_ONE };
	VkBlendOp				colorOperation{ VK_BLEND_OP_ADD };

	VkBlendFactor			sourceAlpha{ VK_BLEND_FACTOR_ONE };
	VkBlendFactor			destinationAlpha{ VK_BLEND_FACTOR_ONE };
	VkBlendOp				alphaOperation{ VK_BLEND_OP_ADD };

	ColorWriteEnableMask	colorWriteMask{ COLOR_WRITE_ENABLE_ALL_MASK };

	bool					blendEnabled{ false };
	bool					separateBlend{ false };
};

struct SpecializationInfo
{
	VkSpecializationInfo		specializationInfo{};
	VkSpecializationMapEntry	specializationData[MAX_SPECIALIZATION_CONSTANTS]{};
	U8*							specializationBuffer{ nullptr };
};

struct ShaderStage
{
	VkShaderStageFlagBits	stage;
	String					entry{ NO_INIT };
	VkShaderModule			module{ nullptr };
	SpecializationInfo		specializationInfo{};
};

struct ShaderOutput
{
	VkFormat format;
};

struct Shader
{
	VkPipelineShaderStageCreateInfo	stageInfos[MAX_SHADER_STAGES]{};
	ShaderStage						stages[MAX_SHADER_STAGES]{};
	U32								shaderCount{ 0 };

	U32								setCount{ 0 };
	DescriptorSetLayout*			setLayouts[MAX_DESCRIPTOR_SETS]{ nullptr };

	U32								vertexStreamCount{ 0 };
	U32								vertexAttributeCount{ 0 };
	VertexStream					vertexStreams[MAX_VERTEX_STREAMS]{};
	VertexAttribute					vertexAttributes[MAX_VERTEX_ATTRIBUTES]{};

	U32								language;

	ShaderOutput					outputs[MAX_IMAGE_OUTPUTS]{};
	U32								outputCount{ 0 };
};

struct Pipeline
{
	bool Create(const String& shaderPath, bool RenderToswapchain);
	void Destroy();

	void SetInput(Texture* texture, U32 binding);
	void SetInput(Buffer* buffer, U32 binding);

	String				name{ NO_INIT };
	U64					handle{ U64_MAX };

	Shader				shader{};
	Renderpass*			renderpass{ nullptr };
	VkPipeline			pipeline{ nullptr };
	VkPipelineLayout	layout{ nullptr };
	VkPipelineBindPoint	bindPoint{ VK_PIPELINE_BIND_POINT_MAX_ENUM };

	Rasterization		rasterization{};
	DepthStencil		depthStencil{};
	BlendState			blendStates[MAX_IMAGE_OUTPUTS]{};
	U8					blendStateCount{ 0 };

	DescriptorSet*		descriptorSets[MAX_SWAPCHAIN_IMAGES][MAX_DESCRIPTOR_SETS];
	U8					descriptorSetCount{ 0 };
	bool				useBindless{ false };

	Buffer*				indexBuffer{ nullptr };
	Buffer*				vertexBuffers[MAX_VERTEX_BUFFERS]{};
	U8					vertexBufferCount{ 0 };

	bool				graphics{ true };
	bool				useDepth{ false };

private:
	bool ParseConfig(const String& data, I64& index);
	bool ParseStage(const String& data, I64& index, DescriptorSetLayoutCreation* setLayoutInfos, VkShaderStageFlagBits stage);
	VkPipelineShaderStageCreateInfo CompileShader(ShaderStage& shaderStage, String& code, const String& name, DescriptorSetLayoutCreation* setLayoutInfos);
	bool ParseSPIRV(U32* code, U64 codeSize, ShaderStage& stage, DescriptorSetLayoutCreation* setLayoutInfos);
	bool CreatePipeline(VkDescriptorSetLayout* vkLayouts);

	static const String& ToStageDefines(VkShaderStageFlagBits value);
	static const String& ToCompilerExtension(VkShaderStageFlagBits value);

	static String CONFIG_BEGIN;
	static String CONFIG_END;
	static String VERT_BEGIN;
	static String VERT_END;
	static String CTRL_BEGIN;
	static String CTRL_END;
	static String EVAL_BEGIN;
	static String EVAL_END;
	static String GEOM_BEGIN;
	static String GEOM_END;
	static String FRAG_BEGIN;
	static String FRAG_END;
	static String COMP_BEGIN;
	static String COMP_END;
};