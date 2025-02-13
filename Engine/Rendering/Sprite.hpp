#pragma once

#include "Resources\ResourceDefines.hpp"

#include "Resources\Mesh.hpp"
#include "Resources\Scene.hpp"
#include "Resources\Component.hpp"

struct NH_API SpriteComponent : public Component
{
	SpriteComponent(const Vector4& color = Vector4One, const ResourceRef<Texture>& texture = nullptr, const Vector4& textureCoords = Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	SpriteComponent(const SpriteComponent& other) noexcept : meshInstance(other.meshInstance) {}
	SpriteComponent(SpriteComponent&& other) noexcept : meshInstance(Move(other.meshInstance)) {}
	SpriteComponent& operator=(SpriteComponent&& other) noexcept
	{
		meshInstance = Move(other.meshInstance);
		return *this;
	}

	virtual void Update(Scene* scene, U32 entityID) final;
	virtual void Load(Scene* scene, U32 entityID) final;
	virtual void Cleanup(Scene* scene, U32 entityID) final {}

	MeshInstance meshInstance;

private:
	static bool Initialize();

	static ResourceRef<Material> material;
	static ResourceRef<Mesh> mesh;
};