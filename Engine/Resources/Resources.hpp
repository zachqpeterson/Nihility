#pragma once

#include "ResourceDefines.hpp"

#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"
#include "Math\Math.hpp"

struct alignas(16) MaterialData
{
	Vector4 base_color_factor;
	Matrix4 model;
	Matrix4 model_inv;

	Vector3 emissive_factor;
	F32   metallic_factor;

	F32   roughness_factor;
	F32   occlusion_factor;
	U32   flags;
};

struct MeshDraw
{
	//BufferHandle index_buffer;
	//BufferHandle position_buffer;
	//BufferHandle tangent_buffer;
	//BufferHandle normal_buffer;
	//BufferHandle texcoord_buffer;
	//
	//BufferHandle material_buffer;
	MaterialData material_data;

	U32 index_offset;
	U32 position_offset;
	U32 tangent_offset;
	U32 normal_offset;
	U32 texcoord_offset;

	U32 count;

	//VkIndexType index_type;

	//DescriptorSetHandle descriptor_set;
};

class NH_API Resources
{
public:
	//static Texture* LoadTexture(String& name);

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

	//static Hashmap<Texture*> textures;

	STATIC_CLASS(Resources);
	friend class Engine;
};