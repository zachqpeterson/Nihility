#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "SpriteComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "ColliderComponent.hpp"
#include "TilemapComponent.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"
#include "Math/Physics.hpp"

struct Scene;

struct NH_API EntityRef
{
	template<class Component, typename... Args>
	void AddComponent(Args... args);

private:
	Scene* scene;
	U32 entityId;
	U32 sceneId;

	friend struct Scene;
};

struct NH_API Scene
{
public:
	bool Create(CameraType type);
	void Destroy();

	EntityRef CreateEntity(Vector2 position = Vector2::Zero, Quaternion2 rotation = Quaternion2::Identity);

private:
	void Update();
	void Render(CommandBuffer commandBuffer) const;

	Vector<Entity> entities;
	Camera camera;

	U32 sceneId;

	static U32 SceneID;

	friend class Renderer;
	friend struct EntityRef;
};

template<class Component, typename... Args>
inline void EntityRef::AddComponent(Args... args)
{
	Entity& entity = scene->entities[entityId];

	if constexpr (IsSame<Component, SpriteComponent>)
	{
		entity.spriteId = SpriteComponent::AddComponent(sceneId, args...);
	}
	else if constexpr (IsSame<Component, RigidBodyComponent>)
	{
		entity.bodyId = RigidBodyComponent::AddComponent(entity.position, entity.rotation, args...);
	}
	else if constexpr (IsSame<Component, ColliderComponent>)
	{
		ColliderComponent::AddComponent(entity.bodyId, args...);
	}
	else if constexpr (IsSame<Component, TilemapComponent>)
	{
		TilemapComponent::AddComponent(sceneId, args...);
	}
}