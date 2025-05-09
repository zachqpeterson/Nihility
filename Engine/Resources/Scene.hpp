#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"
#include "Material.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"

struct NH_API Scene
{
public:
	bool Create(CameraType type);
	void Destroy();

	SpriteInstance* AddSprite(const ResourceRef<Texture>& texture, const Transform& transform = {}, const Vector4& color = Vector4::One, const Vector2& textureCoord = Vector2::Zero, const Vector2& textureScale = Vector2::One);

private:
	void Update();
	void Render(CommandBuffer commandBuffer) const;

	Material spriteMaterial;
	Shader spriteVertexShader;
	Shader spriteFragmentShader;
	Vector<SpriteInstance> spriteInstances;
	Camera camera;
	bool dirtySprites = false;

	friend class Renderer;
};