#pragma once

#include "Resources\Mesh.hpp"
#include "Resources\Scene.hpp"

struct alignas(16) NH_API TilemapData
{
	Vector2 eye;
	Vector2 tileSize;
	U32 width;
	U32 height;
};

struct Camera;

//TODO: Paralax
//TODO: Tilemap Collision
struct NH_API TilemapComponent : Component<TilemapComponent>
{
	TilemapComponent(U16 width, U16 height, Vector2 tileSize);

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;
	virtual void Cleanup(Scene* scene) final;

	Vector2Int MouseToTilemap(const Camera& camera) const;
	U8 AddTile(const ResourceRef<Texture>& texture);
	void ChangeTile(const Vector2Int& pos, U8 id);

private:
	TilemapData data;
	Buffer tiles;
	Buffer legend;
	Buffer staging;
	U8 tileCount = 0;
	Vector2 tileSize;
	U32 width;
	U32 height;
};