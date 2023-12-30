#pragma once

#include "ResourceDefines.hpp"
#include "Rendering\Pipeline.hpp"

struct MeshInstance;
struct Renderpass;
struct Pipeline;
struct CommandBuffer;
struct PipelineInfo;
struct Mesh;

enum RenderStageType
{
	RENDER_STAGE_PRE_PROCESSING,

	RENDER_STAGE_GEOMETRY_OPAQUE,
	RENDER_STAGE_GEOMETRY_TRANSPARENT,
	RENDER_STAGE_PARTICLES,
	RENDER_STAGE_UI_WORLD,

	RENDER_STAGE_POST_PROCESSING,
	RENDER_STAGE_UI_OVERLAY,

	RENDER_STAGE_COUNT,
	RENDER_STAGE_INVALID = I32_MAX
};

struct RenderStage
{
	RenderStageType type{ RENDER_STAGE_INVALID };
	U32				index{ 0 };
	PipelineInfo	info{};
};

struct NH_API RendergraphInfo
{
	RendergraphInfo() { for (U32 i = 0; i < RENDER_STAGE_COUNT; ++i) { for (U32 j = 0; j < MAX_PIPELINES_PER_STAGE; ++j) { stages[i][j] = {}; } } }

	void Destroy() { name.Destroy(); }

	void AddPipeline(const RenderStage& stage);

	RenderStage				stages[RENDER_STAGE_COUNT][MAX_PIPELINES_PER_STAGE]{};

	String					name{};
};

struct NH_API Rendergraph
{
	bool Create(RendergraphInfo& info);
	void Destroy() { name.Destroy(); }

	void Run(CommandBuffer* commandBuffer, const Buffer& vertexBuffer, const Buffer& instanceBuffers, const Buffer& indexBuffer, const Buffer& drawBuffer, const Buffer& countsBuffer);
	void Resize();

	const String& Name() const { return name; }
	U32 InstanceSize(RenderStageType type, U32 index) const;
	U32 VertexSize(RenderStageType type, U32 index) const;
	Pipeline* GetPipeline(RenderStageType type, U32 index);

private:
	Renderpass			renderpasses[RENDER_STAGE_COUNT]{};
	Pipeline			pipelines[RENDER_STAGE_COUNT][MAX_PIPELINES_PER_STAGE]{};

	String				name{};
	HashHandle			handle;

	friend class Resources;
	friend struct Material;
};

struct alignas(16) NH_API MaterialData
{
	U32			diffuseTextureIndex{ U16_MAX };
	U32			armTextureIndex{ U16_MAX };
	U32			normalTextureIndex{ U16_MAX };
	U32			emissivityTextureIndex{ U16_MAX };

	Vector4		baseColorFactor{ Vector4One };
	Vector4		metalRoughFactor{ Vector4One };
	Vector4		emissiveFactor{ Vector4Zero };

	F32			transparency{ 0.0f };
	F32			alphaCutoff{ 0.0f };
	U32			flags{ MATERIAL_FLAG_NONE };
};

struct NH_API Material
{
	void Destroy() { name.Destroy(); }

	String				name{};
	HashHandle			handle;

	RenderStageType		type;
	U32					stageIndex;
	MaterialData		data; //TODO: void*, don't hardcode what a material has to/can pass to a shader
};