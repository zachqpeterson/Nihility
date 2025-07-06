#pragma once

#include "Component.hpp"
#include "Material.hpp"

#include "Rendering/Camera.hpp"

struct TilemapData
{
	Vector2 eye;
	Vector2 tileSize;
	Vector2 offset;
	U32 width;
	U32 height;
};

struct TilemapInstance
{
	F32 depth;
	U32 tileOffset;
};

enum class NH_API TileType
{
	Air,
	Full,
	HalfLeft,
	HalfRight,
	HalfTop,
	HalfBottom,
	SlopeTL,
	SlopeTR,
	SlopeBL,
	SlopeBR
};

class NH_API Tilemap
{
public:
	void SetTile(const ResourceRef<Texture>& texture, const Vector2Int& position, TileType type = TileType::Full);
	Vector2Int ScreenToTilemap(const Camera& camera, const Vector2& position);
	Vector2Int GetDimentions() const;
	const Vector2& GetOffset() const;
	const Vector2& GetTileSize() const;
	const TileType* GetTiles() const;
	bool GetDirty() const;
	void Clean();

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Tilemap> AddTo(const EntityRef& entity, U32 width, U32 height, const Vector2& offset = Vector2::Zero, F32 parallax = 1.0f, F32 depth = 0.0f, const Vector2& tileSize = Vector2::One);

private:
	bool dirty;
	F32 parallax;
	U32 instance;
	Vector2 tileSize;
	Vector2 offset;
	TileType* tileArray;

	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	static DescriptorSet tilemapDescriptor;
	static Material tilemapMaterial;
	static Shader tilemapVertexShader;
	static Shader tilemapFragmentShader;
	static Buffer tilemapData;
	static Buffer tilesData;
	static Vector<TilemapInstance> instanceData;
	static Vector<TilemapData> tilemapDatas;
	static U32 nextOffset;
	static bool initialized;

	COMPONENT(Tilemap);
	friend struct Scene;
	friend struct EntityRef;
};