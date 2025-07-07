#pragma once

#include "Component.hpp"
#include "Material.hpp"

struct SpriteVertex
{
	Vector2 position = Vector2::Zero;
	Vector2 texcoord = Vector2::Zero;
};

struct SpriteInstance
{
	Vector2 position = Vector2::Zero;
	Vector2 scale = Vector2::One;
	Quaternion2 rotation = Quaternion2::Identity;
	Vector4 instColor = Vector4::One;
	Vector2 instTexcoord = Vector2::Zero;
	Vector2 instTexcoordScale = Vector2::One;
	U32 textureIndex = 0;
	U32 spriteIndex = 0;
};

class NH_API Sprite
{
public:
	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Sprite> AddTo(const EntityRef& entity, const ResourceRef<Texture>& texture = nullptr, const Vector4& color = Vector4::One, const Vector2& textureCoord = Vector2::Zero, const Vector2& textureScale = Vector2::One);
	static void RemoveFrom(const EntityRef& entity);

	void SetColor(const Vector4& color);

private:
	U32 instanceIndex = 0;

	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	static Material spriteMaterial;
	static Shader spriteVertexShader;
	static Shader spriteFragmentShader;
	static Vector<SpriteInstance> spriteInstances;
	static bool initialized;

	COMPONENT(Sprite);

	friend struct EntityRef;
};