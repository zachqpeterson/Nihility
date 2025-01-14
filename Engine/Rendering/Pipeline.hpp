#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Resources\Shader\Shader.hpp"

#include "Containers\Vector.hpp"

enum BlendMode
{
	BLEND_MODE_NONE,
	BLEND_MODE_ADD,
	BLEND_MODE_SUB,
};

struct Renderpass;

struct SpecializationEntry
{
	U32 constantID;
	U32 offset;
	U64 size;
};

struct Specialization
{
	U32							mapEntryCount;
	const SpecializationEntry* mapEntries;
	U64							dataSize;
	const void* data;
};

struct NH_API SpecializationInfo
{
	using SizeType = U32;
	static constexpr U64 SIZE = sizeof(SizeType);

	template<typename... Data>
	SpecializationInfo(const Data&... data)
	{
		constexpr U64 count = sizeof...(Data);

		U64 i = 0;
		((specializationBuffer[i++] = data),...);

		for (U64 j = 0; j < count; ++j)
		{
			specializationEntries[j].constantID = (U32)j;
			specializationEntries[j].offset = (U32)(j * SIZE);
			specializationEntries[j].size = 4;
		}

		specializationInfo.dataSize = i * SIZE;
		specializationInfo.mapEntryCount = (U32)i;
		specializationInfo.data = specializationBuffer;
		specializationInfo.mapEntries = specializationEntries;
	}

	SpecializationEntry specializationEntries[MAX_SPECIALIZATION_CONSTANTS]{};
	Specialization specializationInfo{};

	SizeType specializationBuffer[MAX_SPECIALIZATION_CONSTANTS]{};
};

enum DependancyType
{
	DEPENDANCY_NONE = 0,
	DEPENDANCY_RENDER_TARGET,
	DEPENDANCY_DEPTH_TARGET,
	DEPENDANCY_ENTITY_BUFFER,
};

struct Pipeline;

struct NH_API Dependancy
{
	Dependancy(DependancyType type = DEPENDANCY_NONE);
	Dependancy(const ResourceRef<Pipeline>& pipeline, DependancyType type, U8 index = 0);

	ResourceRef<Pipeline> pipeline;
	DependancyType type;
	U8 index = 0;

private:
	U8 descriptor;

	friend struct Pipeline;
	friend struct Scene;
};

struct DrawSet
{
	U32 drawOffset = 0;
	U32 countOffset = 0;
};

struct Renderpass;
struct CommandBuffer;
struct VkPipeline_T;
struct VkBuffer_T;
struct VkPipelineLayout_T;

struct NH_API Pipeline : Resource
{
	void AddDescriptor(const Descriptor& descriptor);
	void AddDependancy(const Dependancy& dependancy);

	void Destroy();

	U8 DescriptorCount() const { return descriptorCount; }

private:
	bool Create(U8 pushConstantCount, PushConstant* pushConstants);
	void Build(Renderpass* renderpass);

	void SetBuffers(Buffer* buffers, Buffer instanceBuffer);

	void Run(CommandBuffer* commandBuffer, Buffer& indexBuffer, Buffer& drawsBuffer, Buffer& countsBuffer);
	void PushDescriptors(CommandBuffer* commandBuffer);
	void PushConstants(CommandBuffer* commandBuffer);

	U32								type = 0; //PipelineType
	I32								renderOrder = 0;
	U32								sizeX = 0;
	U32								sizeY = 0;
	bool							loaded = false;

	Vector<ResourceRef<Shader>>		shaders;
	VkPipeline_T*					pipeline = nullptr;
	VkPipelineLayout_T*				pipelineLayout = nullptr;
	VkDescriptorSetLayout_T*		descriptorSetLayout = nullptr;
	VkDescriptorUpdateTemplate_T*	updateTemplate = nullptr;
	PipelineBindPoint				bindPoint = PIPELINE_BIND_POINT_MAX_ENUM;

	PushConstant					pushConstants[MAX_PUSH_CONSTANTS];
	U8								pushConstantCount = 0;
	U32								pushConstantStages = 0;
	Dependancy						dependancies[MAX_DESCRIPTORS_PER_SET];
	U8								dependancyCount = 0;
	Descriptor						descriptors[MAX_DESCRIPTORS_PER_SET];
	U8								descriptorCount = 0;
	U8								outputCount = 0;

	//Graphics
	U32								renderpassIndex = 0;
	U32								subpassIndex = 0;
	Subpass							subpass;
	I32								clearTypes = 0;
	TopologyMode					topologyMode = TOPOLOGY_MODE_TRIANGLE_LIST;
	CullMode						cullMode = CULL_MODE_NONE;
	FrontFaceMode					frontMode = FRONT_FACE_MODE_COUNTER_CLOCKWISE;
	PolygonMode						fillMode = POLYGON_MODE_FILL;
	BlendMode						blendMode = BLEND_MODE_ADD;
	VertexType						vertexTypes[8];
	U8								vertexBindingCount = 0;

	StencilOperationState			front;
	StencilOperationState			back;
	CompareOp						depthComparison = COMPARE_OP_ALWAYS;
	bool							useVertices = false;
	bool							useIndices = true;
	bool							useInstances = false;
	bool							useBindless = false;
	bool							depthEnable = false;
	bool							depthWriteEnable = false;
	bool							stencilEnable = false;
	bool							depthBiasEnable = false;
	F32								depthBiasConstant = 0.0f;
	F32								depthBiasSlope = 0.0f;
	F32								depthBiasClamp = 0.0f;

	U32								instanceStride = 0;
	U32								vertexCount = 0;

	U32								vertexBufferCount = 0;
	VkBuffer_T*						vertexBuffers[8];
	U64								vertexBufferOffsets[8];
	U32								drawCount = 0;
	U32								drawOffset = 0;
	U32								countOffset = 0;

	//Compute
	U32								localSizeX = 1;
	U32								localSizeY = 1;
	U32								localSizeZ = 1;

	friend class UI;
	friend class Resources;
	friend struct CommandBuffer;
	friend struct Rendergraph;
	friend struct Scene;
};