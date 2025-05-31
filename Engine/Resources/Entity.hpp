#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"

struct NH_API Entity
{
	//TODO: addition info like name potentially

	Vector2 position;
	Vector2 scale;
	Quaternion2 rotation;
};

struct Scene;

struct NH_API EntityRef
{
	EntityRef();
	EntityRef(NullPointer);
	EntityRef(U32 entityId, U32 sceneId);
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
	U32 SceneId() const;

private:
	U32 entityId = U32_MAX;
	U32 sceneId = U32_MAX;

	friend struct Scene;
};