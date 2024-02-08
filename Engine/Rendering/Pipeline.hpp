#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Resources\Shader\Shader.hpp"

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

	SizeType specializationBuffer[MAX_SPECIALIZATION_CONSTANTS]{};
};

struct NH_API PipelineInfo
{
	void Destroy() { name.Destroy(); }
	void AddRenderTarget(Texture* renderTarget) { renderTargets[renderTargetCount++] = renderTarget; }

	String					name{};

	ResourceRef<Shader>		shader{ nullptr };
	U32						type; //PipelineType
	I32						renderOrder;
	bool					resize{ true };

	SpecializationInfo		specialization{};

	U8						renderTargetCount{ 0 };
	ResourceRef<Texture>	renderTargets[MAX_IMAGE_OUTPUTS]{ nullptr };
	ResourceRef<Texture>	depthStencilTarget{ nullptr };

	U32						renderpass{ 0 };
	U32						subpass{ 0 };
};

struct BufferCopy
{
	U64 srcOffset;
	U64 dstOffset;
	U64 size;
};

struct DrawSet
{
	U32 drawOffset{ 0 };
	U32 countOffset{ 0 };
};

struct BufferData;
struct Renderpass;
struct CommandBuffer;
struct VkPipeline_T;

struct NH_API Pipeline
{
	Pipeline() {}
	Pipeline(Pipeline&& other) noexcept;
	Pipeline& operator=(Pipeline&& other) noexcept;

	void Destroy();

	const String& Name() const;

private:
	bool Create(const PipelineInfo& info, Renderpass* renderpass);

	void SetBuffers(const BufferData& buffers);

	void Run(CommandBuffer* commandBuffer);

	String				name{};
	U32					type;

	ResourceRef<Shader>	shader{};
	VkPipeline_T*		pipeline{ nullptr };
	U32					renderpass{ 0 };
	U32					subpass{ 0 };

	U32					vertexCount{ 0 };

	U32					bufferCount{ 0 };
	U8					instanceBuffer{ U8_MAX };
	VkBuffer_T*			vertexBuffers[8]{};
	U64					bufferOffsets[8]{};

	VkBuffer_T*			indexBuffer;
	VkBuffer_T*			drawsBuffer;
	VkBuffer_T*			countsBuffer;

	U32					instanceOffset{ 0 };
	Vector<DrawSet>		drawSets{ 1 };
	U32					drawCount{ 0 };

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	friend class UI;
	friend class Resources;
	friend struct CommandBuffer;
	friend struct Rendergraph;
	friend struct Scene;
};