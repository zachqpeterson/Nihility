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
	F64 radius;
};

struct CapsuleCollider2D : public Collider2D
{
	F64 radius;
	F64 height;
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
	F64 radius;
	//TODO: other

	bool trigger;
	bool kinematic;

	F64 restitution;
	F64 gravityScale;
	F64 density;
	Transform2D* transform;
};

struct PhysicsObject2D
{
	void ApplyForce(const Vector2& f) { force += f; }
	void ApplyTorque(F64 t) { torque += t; }
	void AddVelocity(const Vector2& v) { velocity += v; }
	void AddAngularVelocity(F64 v) { angularVelocity += v; }
	void SetGravityScale(F64 s) { gravityScale = s; }

private:
	U64 id;
	Collider2D* collider;
	Transform2D* transform;

	Vector2 prevPosition;
	F64 prevRotation;

	//secondary
	Vector2 velocity;
	F64 angularVelocity;

	// secondary
	Vector2 force;
	F64 torque;

	bool grounded;

	// constants
	F64 mass;
	F64 massInv;
	F64 inertia;
	F64 inertiaInv;
	F64 friction;
	F64 restitution;
	F64 gravityScale;
	F64 dragCoefficient;
	F64 angularDragCoefficient;
	F64 area;
	U64 layerMask;
	bool kinematic;
	bool freezeRotation;

public:
	// Read-only access
	const Vector2& Velocity = velocity;
	const F64& AngularVelocity = angularVelocity;

	const bool& Grounded = grounded;

	const F64& Mass = mass;
	const F64& Inertia = inertia;
	const F64& Friction = friction;
	const F64& Restitution = restitution;
	const F64& GravityScale = gravityScale;
	const F64& Drag = dragCoefficient;
	const F64& AngularDrag = angularDragCoefficient;
	const F64& Area = area;
	const U64& LayerMask = layerMask;

	friend class Physics;
	friend struct BAH;
};

struct Manifold2D
{
	PhysicsObject2D* a;
	PhysicsObject2D* b;
	Vector2 normal;
	F64 penetration;
};

class NH_API Physics
{
public:
	/// <summary>
	/// TODO: 
	///	Raycasting
	///	Events
	/// </summary>

	static PhysicsObject2D* Create2DPhysicsObject(PhysicsObject2DConfig& config);
	static PhysicsObject3D* Create3DPhysicsObject();
	static bool OverlapRect(const Vector2& boundsX, const Vector2& boundsY, List<PhysicsObject2D*>& results);
	static bool Raycast2D(const Vector2& origin, const Vector2& direction, F32 length, List<PhysicsObject2D*>& results);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update(F64 step);

	static void BroadPhase(struct BAH& tree, List<struct PhysicsObject2D*>& objects, List<Manifold2D>& contacts);
	static void NarrowPhase(List<Manifold2D>& contacts);
	static bool CirclevsCircle(Manifold2D& m);
	static bool AABBvsAABB(Manifold2D& m);
	static bool AABBvsCircle(Manifold2D& m);
	static bool CirclevsAABB(Manifold2D& m);
	static void ResolveCollision(Manifold2D& m);
	static I32 WhichSide(const Vector<Vector2>& vertices, const Vector2& p, const Vector2& d);
	static bool TestIntersection(const Vector<Vector2>& vertices0, const Vector<Vector2>& vertices1, const Vector2& velocity, F32& tFirst, F32& tLast);

	static HashMap<U64, PhysicsObject2D*> physicsObjects2D;
	static HashMap<U64, PhysicsObject3D*> physicsObjects3D;

	static BAH tree;

	static F64 airDensity;
	static F64 gravity;

	friend class Engine;
};