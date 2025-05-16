#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"
#include "Material.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"
#include "Math/Physics.hpp"

struct NH_API EntityId
{
private:
	U32 entityId;
	U32 sceneId;

	friend struct Scene;
};

struct BodyId
{
	I32 index;
	U16 world;
	U16 generation;
};

struct Entity
{
	Vector2 position;
	Quaternion2 rotation;
	BodyId bodyId;
	U32 spriteId = U32_MAX;
};

struct NH_API Scene
{
public:
	bool Create(CameraType type);
	void Destroy();

	EntityId CreateEntity(Vector2 position = Vector2::Zero, Quaternion2 rotation = Quaternion2::Identity);
	void AddSprite(const EntityId& id, const ResourceRef<Texture>& texture, const Vector2& scale = Vector2::One, const Vector4& color = Vector4::One, const Vector2& textureCoord = Vector2::Zero, const Vector2& textureScale = Vector2::One);
	void AddRigidBody(const EntityId& id, BodyType type);
	void AddCollider(const EntityId& id, const Vector2& scale = Vector2::One);

private:
	void Update();
	void Render(CommandBuffer commandBuffer) const;

	Vector<Entity> entities;

	Material spriteMaterial;
	Shader spriteVertexShader;
	Shader spriteFragmentShader;
	Vector<SpriteInstance> spriteInstances;
	Camera camera;

	U32 sceneId;

	static U32 SceneID;

	friend class Renderer;
};