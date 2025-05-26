#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"
#include "Texture.hpp"
#include "Material.hpp"

#include "Math/Math.hpp"
#include "Containers/Vector.hpp"
#include "Rendering/Camera.hpp"

struct alignas(16) NH_API TilemapData
{
	Vector2 eye;
	Vector2 tileSize;
	U32 width;
	U32 height;
};

struct NH_API Tilemap
{
	Buffer tiles;
};

class NH_API TilemapComponent
{
public:
	static void SetTile(const ResourceRef<Texture>& texture, Vector2Int position);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update(U32 sceneId, const Camera& camera, Vector<Entity>& entities);
	static void Render(U32 sceneId, CommandBuffer commandBuffer);

	static void AddScene(U32 sceneId);
	static void RemoveScene(U32 sceneId);
	static void AddComponent(U32 sceneId, U16 width, U16 height, Vector2 tileSize);

	static TilemapData tilemapData; //TODO: per-tilemap
	static DescriptorSet tilemapDescriptor;
	static Material tilemapMaterial;
	static Shader tilemapVertexShader;
	static Shader tilemapFragmentShader;
	static Vector<Vector<Tilemap>> tilemapInstances;
	
	STATIC_CLASS(TilemapComponent);
	friend struct Scene;
	friend struct EntityRef;
};