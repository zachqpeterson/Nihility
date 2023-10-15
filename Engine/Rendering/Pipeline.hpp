#pragma once

#include "RenderingDefines.hpp"
#include "Resources\ResourceDefines.hpp"
#include "Shader.hpp"

struct Renderpass;

union SpecializationData {
	U32 u;
	I32 i;
	F32 f;
	bool b;
};

struct CommandBuffer;

struct SpecializationInfo
{
	using SizeType = U32;
	static constexpr U64 SIZE = sizeof(SizeType);

	template<typename... Data>
	SpecializationInfo(const Data&... data)
	{
		constexpr U64 count = sizeof...(Data);

		U64 i = 0;

		(Memory::Copy(specializationBuffer + i++, &data, sizeof(data)), ...);

		for (U64 j = 0; j < count; ++j)
		{
			specializationConstants[j].constantID = (U32)j;
			specializationConstants[j].offset = (U32)(j * SIZE);
			specializationConstants[j].size = 4;
		}

		specializationInfo.dataSize = i * SIZE;
		specializationInfo.mapEntryCount = (U32)i;
		specializationInfo.pData = specializationBuffer;
		specializationInfo.pMapEntries = specializationConstants;
	}

	VkSpecializationMapEntry	specializationConstants[MAX_SPECIALIZATION_CONSTANTS]{};
	VkSpecializationInfo		specializationInfo{};
	SizeType					specializationBuffer[MAX_SPECIALIZATION_CONSTANTS * SIZE]{};
};

struct PipelineInfo
{
	String				name{};

	U32					vertexBufferSize{ U32_MAX };
	U32					instanceBufferSize{ U32_MAX };
	U32					indexBufferSize{ U32_MAX };
	U32					drawBufferSize{ U32_MAX };

	VkAttachmentLoadOp	colorLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkAttachmentLoadOp	depthLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkAttachmentLoadOp	stencilLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkImageLayout		attachmentFinalLayout{ VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL };
	Texture*			renderTargets[MAX_IMAGE_OUTPUTS]{ nullptr };
	Texture*			depthStencilTarget{ nullptr };
	U8					outputCount{ 0 };

	Shader*				shader{ nullptr };
	Renderpass*			renderpass{ nullptr };
	U32					subpass{ 0 };

	SpecializationInfo	specialization{};

	Descriptor			descriptors[MAX_DESCRIPTORS_PER_SET]{};
	U8					descriptorCount;
};

struct CommandBuffer;

struct Pipeline
{
	U32 UploadIndices(U32 size, const void* data);
	U32 UploadVertices(U32 size, const void* data);
	void UpdateVertices(U32 size, const void* data, U32 regionCount, VkBufferCopy* regions);
	U32 UploadInstances(U32 size, const void* data);
	void UpdateInstances(U32 size, const void* data, U32 regionCount, VkBufferCopy* regions);
	void UploadDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset);
	void UpdateDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset, U32 drawOffset);

	const String& Name() const;

private:
	bool Create(const PipelineInfo& info, const SpecializationInfo& specializationInfo);
	bool CreatePipeline(const SpecializationInfo& specializationInfo);
	void Destroy();

	void Run(CommandBuffer* commandBuffer) const;

	String		name{};
	U64			handle{ U64_MAX };

	Shader*		shader{};
	VkPipeline	pipeline{ nullptr };
	Renderpass* renderpass{ nullptr };
	U32			subpass{ 0 };

	Buffer		vertexBuffer;
	Buffer		instanceBuffer;
	Buffer		indexBuffer;
	Buffer		drawBuffer;

	U32			drawCount{ 0 };
	U32			indexOffset{ 0 };
	U32			vertexOffset{ 0 };
	U32			instanceOffset{ 0 };

	friend class Resources;
	friend class Renderer;
	friend struct CommandBuffer;
	friend struct RenderGraph;
};