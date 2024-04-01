#pragma once

#include "Resources\Component.hpp"
#include "Resources\Mesh.hpp"

struct alignas(16) NH_API TilemapData
{
	Vector2 eye;
	U32 width;
	U32 height;
};

struct NH_API TilemapComponent : Component
{
	TilemapComponent(U16 width, U16 height);

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;

	U8 AddTile(const ResourceRef<Texture>& texture);
	void ChangeTile(U32 x, U32 y, U8 id);

private:
	TilemapData data;
	Buffer tiles;
	Buffer staging;
	Buffer legend;
	U8 tileCount{ 0 };
	U32 width;
	U32 height;
};