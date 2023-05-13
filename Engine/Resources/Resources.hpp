#pragma once

#include "ResourceDefines.hpp"

#include "ResourcePool.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Math\Math.hpp"

struct alignas(16) MaterialData
{
	Vector4 base_colorFactor;
	Matrix4 model;
	Matrix4 modelInv;

	Vector3 emissiveFactor;
	F32   metallicFactor;

	F32   roughnessFactor;
	F32   occlusionFactor;
	U32   flags;
};

struct MeshDraw
{
	BufferHandle indexBuffer;
	BufferHandle positionBuffer;
	BufferHandle tangentBuffer;
	BufferHandle normalBuffer;
	BufferHandle texcoordBuffer;
	
	BufferHandle materialBuffer;
	MaterialData materialData;

	U32 indexOffset;
	U32 positionOffset;
	U32 tangentOffset;
	U32 normalOffset;
	U32 texcoordOffset;

	U32 count;

	VkIndexType indexType;

	DescriptorSetHandle descriptorSet;
};

class NH_API Resources
{
public:
	static Texture* LoadTexture(String& name);



	static bool LoadBinary(const String& name, String& result);

private:
	//Texture Loading
	static bool LoadBMP();
	static bool LoadPNG();
	static bool LoadJPG();
	static bool LoadPSD();
	static bool LoadTIFF();
	static bool LoadTGA();

	static bool Initialize();
	static void Shutdown();

	NH_HEADER_STATIC ResourcePool<Buffer, 4096>				buffers;
	NH_HEADER_STATIC ResourcePool<Texture, 512>				textures;
	NH_HEADER_STATIC ResourcePool<Pipeline, 128>			pipelines;
	NH_HEADER_STATIC ResourcePool<Sampler, 32>				samplers;
	NH_HEADER_STATIC ResourcePool<DesciptorSetLayout, 128>	descriptorSetLayouts;
	NH_HEADER_STATIC ResourcePool<DesciptorSet, 256>		descriptorSets;
	NH_HEADER_STATIC ResourcePool<RenderPass, 256>			renderPasses;
	NH_HEADER_STATIC ResourcePool<ShaderState, 128>			shaders;

	STATIC_CLASS(Resources);
	friend class Engine;
	friend class Renderer;
};