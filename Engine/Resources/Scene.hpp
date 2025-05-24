#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "SpriteComponent.hpp"

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
	void AddRigidBody(const EntityRef& id, BodyType type);
	void AddCollider(const EntityRef& id, const Vector2& scale = Vector2::One);

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
	if constexpr (IsSame<Component, SpriteComponent>)
	{
		scene->entities[entityId].spriteId = SpriteComponent::AddComponent(sceneId, args...);
	}
}