#pragma once

#include "RenderingDefines.hpp"
#include "Shader.hpp"

struct Renderpass;

union SpecializationData {
	U32 u;
	I32 i;
	F32 f;
	bool b;
};

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

	VkAttachmentLoadOp	colorLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkAttachmentLoadOp	depthLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkAttachmentLoadOp	stencilLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkImageLayout		attachmentFinalLayout{ VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL };
	Texture*			outputTextures[MAX_IMAGE_OUTPUTS]{ nullptr };
	Texture*			outputDepth{ nullptr };
	U8					outputCount{ 0 };

	Shader*				shader{ nullptr };
	Renderpass*			renderpass{ nullptr };
	U32					subpass{ 0 };

	SpecializationInfo	specialization{};

	Descriptor			descriptors[MAX_DESCRIPTORS_PER_SET]{};
	U8					descriptorCount;
};

struct Pipeline
{
	bool Create(const PipelineInfo& info, const SpecializationInfo& specializationInfo);
	void Destroy();

	void Resize();

	String				name{};
	U64					handle{ U64_MAX };

	Shader*				shader{};
	VkPipeline			pipeline{ nullptr };
	Renderpass*			renderpass{ nullptr };
	U32					subpass{ 0 };

private:
	bool CreatePipeline(const SpecializationInfo& specializationInfo);
};