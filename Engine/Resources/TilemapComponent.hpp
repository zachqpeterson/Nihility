#pragma once

#include "Component.hpp"
#include "Material.hpp"

#include "Rendering/Camera.hpp"

struct alignas(16) NH_API TilemapData
{
	Vector2 eye;
	Vector2 tileSize;
	U32 width;
	U32 height;
};

class NH_API Tilemap
{
public:
	Buffer tiles;

	void SetTile(const ResourceRef<Texture>& texture, Vector2Int position);

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Tilemap> AddTo(const EntityRef& entity);

private:
	static bool Update(U32 sceneId, Camera& camera, Vector<Entity>& entities);
	static bool Render(U32 sceneId, CommandBuffer commandBuffer);

	static TilemapData tilemapData;
	static DescriptorSet tilemapDescriptor;
	static Material tilemapMaterial;
	static Shader tilemapVertexShader;
	static Shader tilemapFragmentShader;
	static bool initialized;

	COMPONENT(Tilemap, 16);
	friend struct Scene;
	friend struct EntityRef;
};