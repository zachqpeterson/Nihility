#pragma once

#include "Component.hpp"
#include "Material.hpp"

struct SpriteVertex
{
	Vector2 position = Vector2::Zero;
	Vector2 texcoord = Vector2::Zero;
};

class NH_API Sprite
{
public:
	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Sprite> AddTo(const EntityRef& entity, const ResourceRef<Texture>& texture = nullptr, const Vector4& color = Vector4::One, const Vector2& textureCoord = Vector2::Zero, const Vector2& textureScale = Vector2::One);

private:
	Vector2 position = Vector2::Zero;
	Vector2 scale = Vector2::One;
	Quaternion2 rotation = Quaternion2::Identity;
	Vector4 instColor = Vector4::One;
	Vector2 instTexcoord = Vector2::Zero;
	Vector2 instTexcoordScale = Vector2::One;
	U32 textureIndex = 0;

	static bool Update(U32 sceneId, Camera& camera, Vector<Entity>& entities);
	static bool Render(U32 sceneId, CommandBuffer commandBuffer);

	static Material spriteMaterial;
	static Shader spriteVertexShader;
	static Shader spriteFragmentShader;
	static bool initialized;

	COMPONENT(Sprite, 10000);

	friend struct Scene;
	friend struct EntityRef;
};