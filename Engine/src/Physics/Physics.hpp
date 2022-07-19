#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

#include <Containers/HashMap.hpp>

struct Collider
{
	Vector3 center;
	bool trigger;
};

struct PhysicsObject
{
	U64 id;
	Collider collider;
};

struct BoxCollider : public Collider
{
	Vector2 xBounds;
	Vector2 yBounds;
	Vector2 zBounds;
};

struct SphereCollider : public Collider
{
	F32 radius;
};

class NH_API Physics
{
public:
	/// <summary>
	/// TODO: 
	///	Raycasting
	///	Collision detection
	///	Broad-phase culling (bounding volume heirarchy)
	///	Events
	/// </summary>

	static PhysicsObject* CreatePhysicsObject();


private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	static HashMap<U64, PhysicsObject*> physicsObjects;

	friend class Engine;
};