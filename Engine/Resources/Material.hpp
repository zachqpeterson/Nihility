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
	bool hasDefaultOpaque{ false };
	bool hasDefaultTransparent{ false };

	friend struct Rendergraph;
};

struct BufferData;

struct NH_API Rendergraph : public Resource
{
	bool Create(RendergraphInfo& info);
	void Destroy() { name.Destroy(); renderpasses.Destroy(); pipelines.Destroy(); }

	void Run(CommandBuffer* commandBuffer);
	void Resize();

	const String& Name() const { return name; }
	const HashHandle& Handle() const { return handle; }

	void SetBuffers(const BufferData& buffers);
	void AddPreprocessing(Pipeline* pipeline);

	Pipeline* GetPipeline(U8 id);
	Pipeline* DefaultOpaqueMeshPipeline();
	Pipeline* DefaultTransparentMeshPipeline();
	U8 DefaultOpaqueMeshPipelineID() const;
	U8 DefaultTransparentMeshPipelineID() const;

private:
	Vector<Renderpass>	renderpasses{};
	Vector<Pipeline>	pipelines{};
	U8					defaultOpaque{ U8_MAX };
	U8					defaultTransparent{ U8_MAX };

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

	F32			alphaCutoff{ 0.0f };
	U32			flags{ MATERIAL_FLAG_NONE };
};

struct NH_API MaterialInfo
{
	void Destroy() { name.Destroy(); }

	String	name{};

	ResourceRef<Texture> diffuseTexture;
	ResourceRef<Texture> armTexture;
	ResourceRef<Texture> normalTexture;
	ResourceRef<Texture> emissivityTexture;

	Vector4		baseColorFactor{ Vector4One };
	Vector4		metalRoughFactor{ Vector4One };
	Vector4		emissiveFactor{ Vector4Zero };

	F32			alphaCutoff{ 0.0f };
	U32			flags{ MATERIAL_FLAG_NONE };
};

struct NH_API Material : public Resource
{
	const String& Name() const { return name; }
	const HashHandle& Handle() const { return handle; }

private:
	void Destroy() { name.Destroy(); }

	String				name{};
	HashHandle			handle{ U64_MAX };

	Pipeline* meshPipeline;
	//Post-Processing Pipelines

	MaterialData		data{}; //TODO: void*, don't hardcode what a material has to/can pass to a shader

	friend class Resources;
	friend struct Scene;
};