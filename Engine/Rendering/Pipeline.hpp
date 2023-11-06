#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Shader.hpp"

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

		(Memory::Copy(specializationBuffer + i++, &data, sizeof(data)), ...);

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

	SizeType specializationBuffer[MAX_SPECIALIZATION_CONSTANTS * SIZE]{};
};

struct NH_API PipelineInfo
{
	void Destroy() { name.Destroy(); }

	String				name{};

	U32					vertexBufferSize{ U32_MAX };
	U32					instanceBufferSize{ U32_MAX };
	U32					indexBufferSize{ U32_MAX };
	U32					drawBufferSize{ U32_MAX };

	Shader*				shader{ nullptr };

	SpecializationInfo	specialization{};

	U32					renderpass{ 0 };
	U32					subpass{ 0 };

	Descriptor			descriptors[MAX_DESCRIPTORS_PER_SET]{};
	U8					descriptorCount;
};

struct BufferCopy
{
	U64 srcOffset;
	U64 dstOffset;
	U64 size;
};

struct CommandBuffer;
struct VkPipeline_T;
struct Renderpass;

struct NH_API Pipeline
{
	void Destroy();

	U32 UploadIndices(U32 size, const void* data);
	U32 UploadVertices(U32 size, const void* data);
	void UpdateVertices(U32 size, const void* data, U32 regionCount, BufferCopy* regions);
	U32 UploadInstances(U32 size, const void* data);
	void UpdateInstances(U32 size, const void* data, U32 regionCount, BufferCopy* regions);
	void UploadDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset);
	void UpdateDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset, U32 drawOffset);

	const String& Name() const;

private:
	bool Create(const PipelineInfo& info, Renderpass* renderpass);

	void Run(CommandBuffer* commandBuffer) const;

	String			name{};
	U64				handle{ U64_MAX };

	Shader*			shader{};
	VkPipeline_T*	pipeline{ nullptr };
	U32				subpass{ 0 };

	Buffer			vertexBuffer;
	Buffer			instanceBuffer;
	Buffer			indexBuffer;
	Buffer			drawBuffer;
	Buffer			countsBuffer;

	U32				vertexOffset{ 0 };
	U32				instanceOffset{ 0 };
	U32				indexOffset{ 0 };
	U32				drawCount{ 0 };

	friend class UI;
	friend class Resources;
	friend struct CommandBuffer;
	friend struct PipelineGraph;
};