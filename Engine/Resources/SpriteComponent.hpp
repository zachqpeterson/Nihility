#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"
#include "Material.hpp"

class NH_API SpriteComponent
{
private:
	static bool Initialize();
	static void Shutdown();
	static void Update(U32 sceneId, const Vector<Entity>& entities);
	static void Render(U32 sceneId, CommandBuffer commandBuffer);

	static void AddScene(U32 sceneId);
	static void RemoveScene(U32 sceneId);
	static U32 AddComponent(U32 sceneId, const ResourceRef<Texture>& texture, const Vector2& scale = Vector2::One, const Vector4& color = Vector4::One, const Vector2& textureCoord = Vector2::Zero, const Vector2& textureScale = Vector2::One);

	static Material spriteMaterial;
	static Shader spriteVertexShader;
	static Shader spriteFragmentShader;
	static Vector<Vector<SpriteInstance>> spriteInstances;

	STATIC_CLASS(SpriteComponent);
	friend struct Scene;
	friend struct EntityRef;
};