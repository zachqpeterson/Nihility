#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

#include <Containers/HashMap.hpp>
#include <Containers/Vector.hpp>

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

	COLLIDER_TYPE_NONE,
};

struct Collider2D
{
	bool trigger;

	ColliderType2D type;

	Vector2 xBounds;
	Vector2 yBounds;
};

struct RectangleCollider : public Collider2D
{

};

struct CircleCollider : public Collider2D
{
	Vector2 center;
	F32 radius;
};

struct CapsuleCollider2D : public Collider2D
{
	F32 radius;
	F32 height;
	bool yAxis;
};

struct PolygonCollider : public Collider2D
{
	Vector2 center;
	Vector<Vector2> vertices;
	Vector<Vector2> normals;
};

struct PhysicsObject2DConfig
{
	ColliderType2D type;
	//Rectangle
	Vector2 xBounds;
	Vector2 yBounds;
	//Circle
	F32 radius;
	//TODO: other

	bool trigger;
	bool kinematic;

	F32 restitution;
	F32 gravityScale;
	F32 density;
	F32 dragCoefficient;
	Transform2D* transform;
};

struct PhysicsObject2D
{
	U64 id;
	Collider2D* collider;
	Transform2D* transform;

	//secondary
	Vector2 velocity;
	F32 angularVelocity;

	// secondary
	Vector2 force;
	F32 torque;

	// constants
	F32 mass;
	F32 massInv;
	F32 inertia;
	F32 inertiaInv;
	F32 friction;
	F32 restitution;
	F32 gravityScale;
	F32 dragCoefficient;
	F32 area;
	U64 layerMask;
	bool kinematic;
	bool freezeRotation;
};

struct Manifold2D
{
	PhysicsObject2D* a;
	PhysicsObject2D* b;
	Vector2 normal;
	F32 penetration;
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

	static PhysicsObject2D* Create2DPhysicsObject(PhysicsObject2DConfig& config);
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
	static F32 deltaInv;

	friend class Engine;
};