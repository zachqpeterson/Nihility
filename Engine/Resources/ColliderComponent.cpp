#include "ColliderComponent.hpp"

#include "Scene.hpp"

#include "Math/Physics.hpp"

Vector<Collider> Collider::components(1000, {});
Freelist Collider::freeComponents(1000);
bool Collider::initialized = false;

bool Collider::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Collider::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Collider> Collider::AddTo(EntityRef entity)
{
	U32 instanceId;
	Collider& collider = Create(instanceId);
	collider.entityIndex = entity.EntityId();

	Physics::AddCollider({ entity->position + entity->scale, entity->position - entity->scale });

	return { entity.EntityId(), instanceId };
}

bool Collider::Update(Camera& camera, Vector<Entity>& entities)
{
	return false;
}

bool Collider::Render(CommandBuffer commandBuffer)
{
	return false;
}