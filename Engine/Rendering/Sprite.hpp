#pragma once

#include "Resources\ResourceDefines.hpp"

#include "Resources\Component.hpp"
#include "Resources\Mesh.hpp"

struct NH_API SpriteComponent : public Component<SpriteComponent>
{
	SpriteComponent(const Vector4& color = Vector4One, const ResourceRef<Texture>& texture = nullptr, const Vector4& textureCoords = Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	SpriteComponent(const SpriteComponent& other) noexcept : Component<SpriteComponent>(other), meshInstance(other.meshInstance) {}
	SpriteComponent(SpriteComponent&& other) noexcept : Component<SpriteComponent>(Move(other)), meshInstance(Move(other.meshInstance)) {}
	SpriteComponent& operator=(SpriteComponent&& other) noexcept
	{
		Component<SpriteComponent>::operator=(Move(other));
		meshInstance = Move(other.meshInstance);
		return *this;
	}

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;
	virtual void Cleanup(Scene* scene) final {}

	MeshInstance meshInstance;

private:
	static bool Initialize();

	static ResourceRef<Material> material;
	static ResourceRef<Mesh> mesh;
};