#include "Entity.hpp"

#include "World.hpp"

EntityRef::EntityRef() {}

EntityRef::EntityRef(NullPointer) {}

EntityRef::EntityRef(U32 entityId) : entityId(entityId) {}

void EntityRef::Destroy()
{
	entityId = U32_MAX;
}

EntityRef::EntityRef(const EntityRef& other) : entityId(other.entityId) {}

EntityRef::EntityRef(EntityRef&& other) noexcept : entityId(other.entityId)
{
	other.entityId = U32_MAX;
}

EntityRef& EntityRef::operator=(NullPointer)
{
	entityId = U32_MAX;

	return *this;
}

EntityRef& EntityRef::operator=(const EntityRef& other)
{
	entityId = other.entityId;

	return *this;
}

EntityRef& EntityRef::operator=(EntityRef&& other) noexcept
{
	entityId = other.entityId;

	other.entityId = U32_MAX;

	return *this;
}

EntityRef::~EntityRef()
{
	entityId = U32_MAX;
}

Entity* EntityRef::Get()
{
	return &World::GetEntity(entityId);
}

const Entity* EntityRef::Get() const
{
	return &World::GetEntity(entityId);
}

Entity* EntityRef::operator->()
{
	return &World::GetEntity(entityId);
}

const Entity* EntityRef::operator->() const
{
	return &World::GetEntity(entityId);
}

Entity& EntityRef::operator*()
{
	return World::GetEntity(entityId);
}

const Entity& EntityRef::operator*() const
{
	return World::GetEntity(entityId);
}

EntityRef::operator Entity* ()
{
	return &World::GetEntity(entityId);
}

EntityRef::operator const Entity* () const
{
	return &World::GetEntity(entityId);
}

bool EntityRef::operator==(const EntityRef& other) const
{
	return entityId == other.entityId;
}

bool EntityRef::Valid() const
{
	return entityId != U32_MAX;
}

EntityRef::operator bool() const
{
	return entityId != U32_MAX;
}

U32 EntityRef::EntityId() const
{
	return entityId;
}