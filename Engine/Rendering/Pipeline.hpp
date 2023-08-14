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
	SpecializationInfo() {}

	template <U64 Count>
	constexpr SpecializationInfo(const SpecializationData(&data)[Count])
	{
		U32 i = 0;
		for (; i < Count; ++i)
		{
			specializationConstants->constantID = i;
			specializationConstants->offset = i * sizeof(U32);
			specializationConstants->size = 4;
			Memory::Copy(specializationBuffer + i * sizeof(U32), data + i, sizeof(U32));
			BreakPoint;
		}

		specializationInfo.dataSize = i * sizeof(U32);
		specializationInfo.mapEntryCount = i;
		specializationInfo.pData = specializationBuffer;
		specializationInfo.pMapEntries = specializationConstants;
	}

	VkSpecializationMapEntry	specializationConstants[MAX_SPECIALIZATION_CONSTANTS]{};
	VkSpecializationInfo		specializationInfo{};
	U8*							specializationBuffer[MAX_SPECIALIZATION_CONSTANTS * sizeof(U32)]{};
};

struct PipelineInfo
{
	String				name{ NO_INIT };

	VkAttachmentLoadOp	colorLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkAttachmentLoadOp	depthLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	VkAttachmentLoadOp	stencilLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };

	Shader*				shader{ nullptr };
	Renderpass*			renderpass{ nullptr };

	SpecializationInfo	specialization{};

	Descriptor			descriptors[MAX_DESCRIPTORS_PER_SET]{};
	U8					descriptorCount;
};

struct Pipeline
{
	bool Create();
	void Destroy();

	void Update();
	void Resize();

	void AddDescriptor(const Descriptor& descriptor);

	String				name{ NO_INIT };
	U64					handle{ U64_MAX };

	Shader*				shader{};
	Renderpass*			renderpass{ nullptr };
	VkPipeline			pipeline{ nullptr };

	Descriptor			descriptors[MAX_DESCRIPTORS_PER_SET]{};
	U8					descriptorCount;

private:
	bool CreatePipeline();
};