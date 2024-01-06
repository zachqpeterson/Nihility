#pragma once

#include "ResourceDefines.hpp"
#include "Rendering\Pipeline.hpp"

struct MeshInstance;
struct Renderpass;
struct Pipeline;
struct CommandBuffer;
struct PipelineInfo;
struct Mesh;

struct NH_API RendergraphInfo
{
	void Destroy() { name.Destroy(); }

	void AddPipeline(const PipelineInfo& pipeline);

	String					name{};

private:
	Vector<PipelineInfo>	pipelines{};

	friend struct Rendergraph;
};

struct BufferData;

struct NH_API Rendergraph
{
	bool Create(RendergraphInfo& info);
	void Destroy() { name.Destroy(); renderpasses.Destroy(); pipelines.Destroy(); }

	void Run(CommandBuffer* commandBuffer, PipelineGroup* groups);
	void Resize();

	const String& Name() const { return name; }

	void SetBuffers(const BufferData& buffers);

private:
	Vector<Renderpass>	renderpasses{};
	Vector<Pipeline>	pipelines{};

	String				name{};
	HashHandle			handle{ U64_MAX };

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
	HashHandle			handle{ U64_MAX };

	MaterialType		type{};
	MaterialData		data{}; //TODO: void*, don't hardcode what a material has to/can pass to a shader
};