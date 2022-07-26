#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

#include <Containers/HashMap.hpp>

/// <summary>
/// TODO: 
/// 
/// ---3D---
/// Box
/// Sphere
/// Capsule
/// Cylinder
///	Polyhedron
/// 
/// ---2D---
/// Capsule
/// Polygon
/// 
/// </summary>

// ------3D------

struct Collider3D
{
	bool trigger;
};

struct PhysicsObject3D
{
	U64 id;
	Collider3D* collider;
};

// ------2D------

enum ColliderType2D
{
	COLLIDER_TYPE_RECTANGLE,
	COLLIDER_TYPE_CIRCLE,
	COLLIDER_TYPE_CAPSULE,
	COLLIDER_TYPE_POLYGON,
};

struct Collider2D
{
	bool trigger;

	ColliderType2D type;

	virtual Vector2 XBounds() = 0;
	virtual Vector2 YBounds() = 0;
};

struct RectangleCollider : public Collider2D
{
	Vector2 xBounds;
	Vector2 yBounds;

	Vector2 XBounds() final { return xBounds; }
	Vector2 YBounds() final { return yBounds; }
};

struct CircleCollider : public Collider2D
{
	F32 radius;

	Vector2 XBounds() final { return { -radius, radius }; }
	Vector2 YBounds() final { return { -radius, radius }; }
};

struct CapsuleCollider : public Collider2D
{
	Vector2 XBounds() final { return { 0.0f, 0.0f }; }
	Vector2 YBounds() final { return { 0.0f, 0.0f }; }
};

struct PolygonCollider : public Collider2D
{
	Vector2 XBounds() final { return { 0.0f, 0.0f }; }
	Vector2 YBounds() final { return { 0.0f, 0.0f }; }
};

struct PhysicsObject2D
{
	U64 id;
	Collider2D* collider;

	//primary
	Vector2 position; //TODO: Transform
	Vector2 momentum;

	F32 rotation; //TODO: Transform
	F32 angularMomentum;

	// secondary
	Vector2 velocity;
	Vector2 force;

	F32 spin;
	F32 angularVelocity;

	// constants
	F32 mass;
	F32 massInv;
	F32 inertia;
	F32 inertiaInv;
	F32 restitution;
	F32 gravityScale;
	F32 dragCoefficient;
	F32 area;
};

struct Manifold2D
{
	PhysicsObject2D* a;
	PhysicsObject2D* b;
	F32 penetration;
	Vector2 normal;
};

class NH_API Physics
{
public:
	/// <summary>
	/// TODO: 
	///	Raycasting
	///	Broad-phase culling (bounding volume heirarchy)
	///	Events
	/// </summary>

	static PhysicsObject2D* Create2DPhysicsObject(ColliderType2D colliderType, F32 mass, F32 restitution);
	static PhysicsObject3D* Create3DPhysicsObject();


private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	static void BroadPhase(struct BAH& tree, List<struct PhysicsObject2D*>& objects, List<Manifold2D>& contacts);
	static void NarrowPhase(List<Manifold2D>& contacts);
	static bool CirclevsCircle(Manifold2D& m);
	static bool AABBvsAABB(Manifold2D& m);
	static bool AABBvsCircle(Manifold2D& m);
	static bool CirclevsAABB(Manifold2D& m);
	static void ResolveCollision(Manifold2D& m);

	static HashMap<U64, PhysicsObject2D*> physicsObjects2D;
	static HashMap<U64, PhysicsObject3D*> physicsObjects3D;

	static F32 airPressure;
	static F32 gravity;

	friend class Engine;
};