#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"

struct NH_API Entity
{
	//TODO: addition info like name potentially

	Vector2 position;
	Vector2 scale;
	Quaternion2 rotation;

	Vector2 prevPosition;
	Quaternion2 prevRotation;
};

struct NH_API EntityRef
{
	EntityRef();
	EntityRef(NullPointer);
	EntityRef(U32 entityId);
	void Destroy();

	EntityRef(const EntityRef& other);
	EntityRef(EntityRef&& other) noexcept;
	EntityRef& operator=(NullPointer);
	EntityRef& operator=(const EntityRef& other);
	EntityRef& operator=(EntityRef&& other) noexcept;
	~EntityRef();

	Entity* Get();
	const Entity* Get() const;
	Entity* operator->();
	const Entity* operator->() const;
	Entity& operator*();
	const Entity& operator*() const;
	operator Entity* ();
	operator const Entity* () const;

	bool operator==(const EntityRef& other) const;

	bool Valid() const;
	operator bool() const;

	U32 EntityId() const;

private:
	U32 entityId = U32_MAX;

	friend class World;
};