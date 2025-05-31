#include "Entity.hpp"

#include "Scene.hpp"

EntityRef::EntityRef() {}

EntityRef::EntityRef(NullPointer) {}

EntityRef::EntityRef(U32 entityId, U32 sceneId) : entityId(entityId), sceneId(sceneId) {}

void EntityRef::Destroy()
{
	entityId = U32_MAX;
	sceneId = U32_MAX;
}

EntityRef::EntityRef(const EntityRef& other) : entityId(other.entityId), sceneId(other.sceneId) {}

EntityRef::EntityRef(EntityRef&& other) noexcept : entityId(other.entityId), sceneId(other.sceneId)
{
	other.entityId = U32_MAX;
	other.sceneId = U32_MAX;
}

EntityRef& EntityRef::operator=(NullPointer)
{
	entityId = U32_MAX;
	sceneId = U32_MAX;

	return *this;
}

EntityRef& EntityRef::operator=(const EntityRef& other)
{
	entityId = other.entityId;
	sceneId = other.sceneId;

	return *this;
}

EntityRef& EntityRef::operator=(EntityRef&& other) noexcept
{
	entityId = other.entityId;
	sceneId = other.sceneId;

	other.entityId = U32_MAX;
	other.sceneId = U32_MAX;

	return *this;
}

EntityRef::~EntityRef()
{
	entityId = U32_MAX;
	sceneId = U32_MAX;
}

Entity* EntityRef::Get()
{
	return Scene::GetEntity(sceneId, entityId);
}

const Entity* EntityRef::Get() const
{
	return Scene::GetEntity(sceneId, entityId);
}

Entity* EntityRef::operator->()
{
	return Scene::GetEntity(sceneId, entityId);
}

const Entity* EntityRef::operator->() const
{
	return Scene::GetEntity(sceneId, entityId);
}

Entity& EntityRef::operator*()
{
	return *Scene::GetEntity(sceneId, entityId);
}

const Entity& EntityRef::operator*() const
{
	return *Scene::GetEntity(sceneId, entityId);
}

EntityRef::operator Entity* ()
{
	return Scene::GetEntity(sceneId, entityId);
}

EntityRef::operator const Entity* () const
{
	return Scene::GetEntity(sceneId, entityId);
}

bool EntityRef::operator==(const EntityRef& other) const
{
	return entityId == other.entityId && sceneId == other.sceneId;
}

bool EntityRef::Valid() const
{
	return Scene::GetEntity(sceneId, entityId);
}

EntityRef::operator bool() const
{
	return Scene::GetEntity(sceneId, entityId);
}

U32 EntityRef::EntityId() const
{
	return entityId;
}

U32 EntityRef::SceneId() const
{
	return sceneId;
}