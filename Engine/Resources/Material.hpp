#pragma once

#include "ResourceDefines.hpp"
#include "Rendering\Pipeline.hpp"

struct Pipeline;

struct alignas(16) NH_API MaterialData
{
	U32			diffuseTextureIndex = U16_MAX;
	U32			armTextureIndex = U16_MAX;
	U32			normalTextureIndex = U16_MAX;
	U32			emissivityTextureIndex = U16_MAX;

	Vector4		baseColorFactor = Vector4One;
	Vector4		metalRoughFactor = Vector4One;
	Vector4		emissiveFactor = Vector4Zero;

	F32			alphaCutoff = 0.0f;
	U32			flags = MATERIAL_FLAG_NONE;
};

struct NH_API MaterialEffect : public Resource
{
	Vector<ResourceRef<Pipeline>>	processing;
};

struct NH_API MaterialInfo
{
	void Destroy() { name.Destroy(); }

	String						name;
	ResourceRef<MaterialEffect> effect;

	ResourceRef<Texture>		diffuseTexture;
	ResourceRef<Texture>		armTexture;
	ResourceRef<Texture>		normalTexture;
	ResourceRef<Texture>		emissivityTexture;

	Vector4						baseColorFactor = Vector4One;
	Vector4						metalRoughFactor = Vector4One;
	Vector4						emissiveFactor = Vector4Zero;

	F32							alphaCutoff = 0.0f;
	U32							flags = MATERIAL_FLAG_NONE;
};

struct NH_API Material : public Resource
{
private:
	ResourceRef<MaterialEffect>		effect;

	MaterialData					data;

	friend class Resources;
	friend struct Scene;
};