#pragma once
#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Containers\String.hpp"
#include "Containers\Pool.hpp"

static constexpr U8	MAX_SHADER_STAGES = 5;				// Maximum simultaneous shader stages, applicable to all different type of pipelines
static constexpr U8 MAX_DESCRIPTOR_SETS = 8;
static constexpr U8	MAX_DESCRIPTORS_PER_SET = 16;		// Maximum list elements for both descriptor set layout and descriptor sets
static constexpr U8	MAX_VERTEX_STREAMS = 16;			// Maximum vertex streams a shader can have
static constexpr U8	MAX_VERTEX_ATTRIBUTES = 16;			// Maximum vertex attributes a shader can have
static constexpr U8	MAX_PUSH_CONSTANTS = 8;				// Maximum number of push constants a shader can have
static constexpr U8	MAX_PUSH_CONSTANT_SIZE = 128;		// Maximum size of all push constants a shader can have

enum ClearType
{
	CLEAR_TYPE_COLOR = 1,
	CLEAR_TYPE_DEPTH = 2,
	CLEAR_TYPE_STENCIL = 4,
};

enum CullMode
{
	CULL_MODE_NONE = 0,
	CULL_MODE_FRONT_BIT = 0x00000001,
	CULL_MODE_BACK_BIT = 0x00000002,
	CULL_MODE_FRONT_AND_BACK = 0x00000003,
	CULL_MODE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
};

enum FrontFaceMode
{
	FRONT_FACE_MODE_COUNTER_CLOCKWISE = 0,
	FRONT_FACE_MODE_CLOCKWISE = 1,
};

enum PolygonMode
{
	POLYGON_MODE_FILL = 0,
	POLYGON_MODE_LINE = 1,
	POLYGON_MODE_POINT = 2,
	POLYGON_MODE_FILL_RECTANGLE_NV = 1000153000,
};

enum TopologyMode
{
	TOPOLOGY_MODE_POINT_LIST = 0,
	TOPOLOGY_MODE_LINE_LIST = 1,
	TOPOLOGY_MODE_LINE_STRIP = 2,
	TOPOLOGY_MODE_TRIANGLE_LIST = 3,
	TOPOLOGY_MODE_TRIANGLE_STRIP = 4,
	TOPOLOGY_MODE_TRIANGLE_FAN = 5,
	TOPOLOGY_MODE_LINE_LIST_WITH_ADJACENCY = 6,
	TOPOLOGY_MODE_LINE_STRIP_WITH_ADJACENCY = 7,
	TOPOLOGY_MODE_TRIANGLE_LIST_WITH_ADJACENCY = 8,
	TOPOLOGY_MODE_TRIANGLE_STRIP_WITH_ADJACENCY = 9,
	TOPOLOGY_MODE_PATCH_LIST = 10
};

enum StencilOp
{
	STENCIL_OP_KEEP = 0,
	STENCIL_OP_ZERO = 1,
	STENCIL_OP_REPLACE = 2,
	STENCIL_OP_INCREMENT_AND_CLAMP = 3,
	STENCIL_OP_DECREMENT_AND_CLAMP = 4,
	STENCIL_OP_INVERT = 5,
	STENCIL_OP_INCREMENT_AND_WRAP = 6,
	STENCIL_OP_DECREMENT_AND_WRAP = 7,
	STENCIL_OP_MAX_ENUM = 0x7FFFFFFF
};

enum CompareOp
{
	COMPARE_OP_NEVER = 0,
	COMPARE_OP_LESS = 1,
	COMPARE_OP_EQUAL = 2,
	COMPARE_OP_LESS_OR_EQUAL = 3,
	COMPARE_OP_GREATER = 4,
	COMPARE_OP_NOT_EQUAL = 5,
	COMPARE_OP_GREATER_OR_EQUAL = 6,
	COMPARE_OP_ALWAYS = 7,
	COMPARE_OP_MAX_ENUM = 0x7FFFFFFF
};

enum ShaderStageType
{
	SHADER_STAGE_TYPE_VERTEX_BIT = 0x00000001,
	SHADER_STAGE_TYPE_TESSELLATION_CONTROL_BIT = 0x00000002,
	SHADER_STAGE_TYPE_TESSELLATION_EVALUATION_BIT = 0x00000004,
	SHADER_STAGE_TYPE_GEOMETRY_BIT = 0x00000008,
	SHADER_STAGE_TYPE_FRAGMENT_BIT = 0x00000010,
	SHADER_STAGE_TYPE_COMPUTE_BIT = 0x00000020,
	SHADER_STAGE_TYPE_ALL_GRAPHICS = 0x0000001F,
	SHADER_STAGE_TYPE_ALL = 0x7FFFFFFF,
	SHADER_STAGE_TYPE_RAYGEN_BIT_KHR = 0x00000100,
	SHADER_STAGE_TYPE_ANY_HIT_BIT_KHR = 0x00000200,
	SHADER_STAGE_TYPE_CLOSEST_HIT_BIT_KHR = 0x00000400,
	SHADER_STAGE_TYPE_MISS_BIT_KHR = 0x00000800,
	SHADER_STAGE_TYPE_INTERSECTION_BIT_KHR = 0x00001000,
	SHADER_STAGE_TYPE_CALLABLE_BIT_KHR = 0x00002000,
	SHADER_STAGE_TYPE_TASK_BIT_EXT = 0x00000040,
	SHADER_STAGE_TYPE_MESH_BIT_EXT = 0x00000080,
	SHADER_STAGE_TYPE_SUBPASS_SHADING_BIT_HUAWEI = 0x00004000,
	SHADER_STAGE_TYPE_CLUSTER_CULLING_BIT_HUAWEI = 0x00080000,
	SHADER_STAGE_TYPE_RAYGEN_BIT_NV = SHADER_STAGE_TYPE_RAYGEN_BIT_KHR,
	SHADER_STAGE_TYPE_ANY_HIT_BIT_NV = SHADER_STAGE_TYPE_ANY_HIT_BIT_KHR,
	SHADER_STAGE_TYPE_CLOSEST_HIT_BIT_NV = SHADER_STAGE_TYPE_CLOSEST_HIT_BIT_KHR,
	SHADER_STAGE_TYPE_MISS_BIT_NV = SHADER_STAGE_TYPE_MISS_BIT_KHR,
	SHADER_STAGE_TYPE_INTERSECTION_BIT_NV = SHADER_STAGE_TYPE_INTERSECTION_BIT_KHR,
	SHADER_STAGE_TYPE_CALLABLE_BIT_NV = SHADER_STAGE_TYPE_CALLABLE_BIT_KHR,
	SHADER_STAGE_TYPE_TASK_BIT_NV = SHADER_STAGE_TYPE_TASK_BIT_EXT,
	SHADER_STAGE_TYPE_MESH_BIT_NV = SHADER_STAGE_TYPE_MESH_BIT_EXT,
	SHADER_STAGE_TYPE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
};

enum PipelineBindPoint
{
	PIPELINE_BIND_POINT_GRAPHICS = 0,
	PIPELINE_BIND_POINT_COMPUTE = 1,
	PIPELINE_BIND_POINT_RAY_TRACING_KHR = 1000165000,
	PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI = 1000369003,
	PIPELINE_BIND_POINT_RAY_TRACING_NV = PIPELINE_BIND_POINT_RAY_TRACING_KHR,
	PIPELINE_BIND_POINT_MAX_ENUM = 0x7FFFFFFF
};

enum DescriptorType
{
	DESCRIPTOR_TYPE_BUFFER,
	DESCRIPTOR_TYPE_IMAGE,
};

struct Texture;
struct VkBuffer_T;
struct VkSampler_T;
struct VkImageView_T;
struct VkDescriptorSetLayout_T;
struct VkDescriptorUpdateTemplate_T;
enum VkShaderStageFlagBits;

struct NH_API Descriptor
{
	union
	{
		struct Buffer
		{
			VkBuffer_T* buffer;
			U64 offset;
			U64 range;
		} bufferInfo;

		struct Image
		{
			VkSampler_T* sampler;
			VkImageView_T* imageView;
			ImageLayout imageLayout;
		} imageInfo;
	};

	Descriptor() {}
	Descriptor(VkBuffer_T* buffer, U64 offset = 0, U64 range = ~0ULL);
	Descriptor(VkImageView_T* imageView, ImageLayout imageLayout, VkSampler_T* sampler = nullptr);
	Descriptor(const ResourceRef<Texture>& texture);
};

struct StencilOperationState
{
	StencilOp	fail{ STENCIL_OP_KEEP };
	StencilOp	pass{ STENCIL_OP_KEEP };
	StencilOp	depthFail{ STENCIL_OP_KEEP };
	CompareOp	compare{ COMPARE_OP_ALWAYS };
	U32			compareMask{ 0xff };
	U32			writeMask{ 0xff };
	U32			reference{ 0xff };
};

struct ShaderStage
{
	String			entryPoint{};
	ShaderStageType	stage{ SHADER_STAGE_TYPE_FLAG_BITS_MAX_ENUM };

	bool			usePushConstants{ false };
};

struct DescriptorSetLayout;
struct Descriptor;
struct CommandBuffer;
struct VkPipelineLayout_T;
struct VkDescriptorPool_T;
struct VkDescriptorSet_T;
struct VkDescriptorSetLayout_T;
struct VkPushConstantRange;
struct VkPipelineShaderStageCreateInfo;
struct VkGraphicsPipelineCreateInfo;
struct VkPipelineVertexInputStateCreateInfo;
struct VkPipelineColorBlendStateCreateInfo;
struct VkComputePipelineCreateInfo;
struct Specialization;
struct ShaderInfo;

struct NH_API Shader : public Resource
{
	bool Create(const String& shaderPath, U8 pushConstantCount, PushConstant* pushConstants);
	void Destroy();

	void FillOutShaderInfo(VkGraphicsPipelineCreateInfo& pipelineInfo, VkPipelineVertexInputStateCreateInfo& vertexInput,
		VkPipelineColorBlendStateCreateInfo& colorBlending, const Specialization* specialization);
	void FillOutShaderInfo(VkComputePipelineCreateInfo& pipelineInfo, const Specialization* specialization);

	void AddDescriptor(const Descriptor& descriptor);
	void PushDescriptors(CommandBuffer* commandBuffer);

	String								name{};
	U64									handle{ U64_MAX };

	String								entryPoint;
	PipelineBindPoint					bindPoint{ PIPELINE_BIND_POINT_MAX_ENUM };
	VkPipelineLayout_T* pipelineLayout{ nullptr };
	ShaderStage							stages[MAX_SHADER_STAGES]{};

	U32									pushConstantStages{ 0 }; //VkShaderStageFlags
	PushConstant						pushConstants[MAX_PUSH_CONSTANTS]{};
	U8									pushConstantCount{ 0 };

	DescriptorSetLayout* setLayout{};
	Descriptor							descriptors[MAX_DESCRIPTORS_PER_SET]; //TODO: Descriptors should probably be per-pipeline
	U8									descriptorCount{ 0 };
	bool								useSet0{ false };
	bool								useBindless{ false };

	//Graphics
	StencilOperationState				front{};
	StencilOperationState				back{};
	CompareOp							depthComparison{ COMPARE_OP_ALWAYS };
	bool								depthEnable{ false };
	bool								depthWriteEnable{ false };
	bool								stencilEnable{ false };
	bool								depthBiasEnable{ false };
	F32									depthBiasConstant{ 0.0f };
	F32									depthBiasSlope{ 0.0f };
	F32									depthBiasClamp{ 0.0f };

	I32									clearTypes{ 0 };
	TopologyMode						topologyMode{ TOPOLOGY_MODE_TRIANGLE_LIST };
	CullMode							cullMode{ CULL_MODE_NONE };
	FrontFaceMode						frontMode{ FRONT_FACE_MODE_COUNTER_CLOCKWISE };
	PolygonMode							fillMode{ POLYGON_MODE_FILL };

	bool								useVertices{ true };
	bool								useInstancing{ false };
	bool								useIndexing{ true };
	U8									vertexBindingCount{ 0 };
	VertexType							vertexTypes[8];
	U8									instanceBinding{ U8_MAX };
	U8									instanceLocation{ U8_MAX };
	U32									instanceStride{ 0 };
	U32									vertexCount{ 0 };

	U8									outputCount{ 0 };
	Subpass								subpass{};

	//Compute
	U32									localSizeX{ 1 };
	U32									localSizeY{ 1 };
	U32									localSizeZ{ 1 };

private:
	bool ParseConfig(const String& data, I64& index);
	bool ParseStage(const String& data, I64& index, VkShaderStageFlagBits stage);
	VkPipelineShaderStageCreateInfo CompileShader(ShaderStage& shaderStage, String& code, const String& name);
	VkPipelineShaderStageCreateInfo ParseSPIRV(U32* code, U32 codeSize, ShaderStage& stage);

	static const String& ToStageDefines(VkShaderStageFlagBits value);
	static const String& ToCompilerExtension(VkShaderStageFlagBits value);

	ShaderInfo* shaderInfo{};

	static bool Initialize();
	static void Shutdown();

	static VkDescriptorSetLayout_T* dummyDescriptorSetLayout;

	static Pool<DescriptorSetLayout, 256>	descriptorSetLayouts;
	static VkDescriptorPool_T* bindlessDescriptorPool;
	static VkDescriptorSet_T* bindlessDescriptorSet;
	static DescriptorSetLayout* bindlessDescriptorSetLayout;
	static constexpr U32					maxBindlessResources{ 1024 };
	static constexpr U32					bindlessTextureBinding{ 10 };

	friend class Resources;
	friend class Renderer;
};