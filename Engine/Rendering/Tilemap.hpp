#pragma once

#include "Resources\Component.hpp"
#include "Resources\Mesh.hpp"

struct alignas(16) NH_API TilemapData
{
	Vector2 eye;
	Vector2 tileSize;
	U32 width;
	U32 height;
};

struct NH_API TilemapComponent : Component
{
	TilemapComponent(U16 width, U16 height, Vector2 tileSize);
	//TODO: Cleanup on exit

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;
	virtual void Cleanup(Scene* scene) final;

	Vector2Int WorldToTilemap(const Vector2& worldPos) const;
	U8 AddTile(const ResourceRef<Texture>& texture);
	void ChangeTile(const Vector2Int& pos, U8 id);

private:
	TilemapData data;
	Buffer tiles;
	Buffer legend;
	Buffer staging;
	U8 tileCount{ 0 };
	Vector2 tileSize;
	U32 width;
	U32 height;
};