#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

#include <Containers/HashMap.hpp>
#include <Containers/Vector.hpp>
#include <Containers/Array.hpp>

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

typedef bool(*Collision2DFn)(struct Contact2D& c);

enum Collider2DType
{
	POLYGON_COLLIDER,
	CIRCLE_COLLIDER,

	COLLIDER_2D_MAX
};

struct Box
{
	Vector2 xBounds;
	Vector2 yBounds;

	Vector2 Center() const
	{
		return Vector2(xBounds.y + xBounds.x, yBounds.y + yBounds.x) * 0.5f;
	}

	Vector2 Extents() const
	{
		return Vector2(xBounds.y - xBounds.x, yBounds.y - yBounds.x) * 0.5f;
	}

	bool Contains(const Box& b) const
	{
		return b.xBounds.x <= xBounds.y && b.xBounds.y >= xBounds.x && b.yBounds.x <= yBounds.y && b.yBounds.y >= yBounds.x;
	}

	F32 GetPerimeter() const
	{
		F32 wx = xBounds.y - xBounds.x;
		F32 wy = yBounds.y - yBounds.x;
		return 2.0f * (wx + wy);
	}

	void Combine(const Box& box)
	{
		xBounds.x = Math::Min(xBounds.x, box.xBounds.x);
		xBounds.y = Math::Max(xBounds.y, box.xBounds.y);
		yBounds.x = Math::Min(yBounds.x, box.xBounds.x);
		yBounds.y = Math::Max(yBounds.y, box.xBounds.y);
	}

	void Combine(const Box& box0, const Box& box1)
	{
		xBounds.x = Math::Min(box0.xBounds.x, box1.xBounds.x);
		xBounds.y = Math::Max(box0.xBounds.y, box1.xBounds.y);
		yBounds.x = Math::Min(box0.yBounds.x, box1.xBounds.x);
		yBounds.y = Math::Max(box0.yBounds.y, box1.xBounds.y);
	}

	Box operator+(const Vector2& v) const { return{ xBounds + v.x, yBounds + v.y }; }
};

struct Collider2D
{
	Collider2DType type;

	bool trigger;

	Box box;
};

struct PolygonCollider : public Collider2D
{
	Vector<Vector2> shape;
};

struct CircleCollider : public Collider2D
{
	Vector2 offset;
	F64 radius;
};

struct PhysicsObject2DConfig
{
	Collider2DType type;
	F64 radius;
	Vector2 offset;

	Vector<Vector2> shape;

	bool trigger;
	bool kinematic;

	F64 restitution;
	F64 gravityScale;
	F64 density;
	Transform2D* transform;
};

struct Pair
{
	I32 proxyIDA;
	I32 proxyIDB;
};

struct RayCastInput
{
	Vector2 p1, p2;
	F32 maxFraction;
};

struct Simplex
{
	Vector2 a;
	Vector2 b;
	Vector2 c;
};

struct Edge
{
	Vector2 vertex0;
	Vector2 vertex1;
	Vector2 normal;
	F32 distance;
	U32 index;
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
	I32 proxyID;
	Collider2D* collider;
	Transform2D* transform;

	Vector2 prevPosition;
	F64 prevRotation;

	//secondary
	Vector2 velocity;
	Vector2 oneTimeVelocity;
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
	const U64& ID = id;

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
	const bool& Kinematic = kinematic;

	friend class Physics;
	friend class ContactManager;
	friend class Broadphase;
	friend struct Tree;
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
	static bool Raycast2D(const Vector2& origin, const Vector2& direction, F32 length, List<PhysicsObject2D*>& results);
	static bool TestOverlap(const Box& a, const Box& b) { return a.xBounds.x <= b.xBounds.y && a.xBounds.y >= b.xBounds.x && a.yBounds.x <= b.yBounds.y && a.yBounds.y >= b.yBounds.x; }

private:
	static bool Initialize();
	static void Shutdown();
	static void Update(F64 step);

	static void BroadPhase();
	static void NarrowPhase();
	static bool CircleVsCircle(Contact2D& c);
	static bool PolygonVsPolygon(Contact2D& c);
	static bool PolygonVsCircle(Contact2D& c);
	static bool CircleVsPolygon(Contact2D& c);
	static void ResolveCollision(Contact2D& c);

	static bool ContainsOrigin(List<Vector2>& simplex, Vector2& direction);
	static Vector2 FarthestPoint(const Vector<Vector2>& shape, const Vector2& direction);
	static Edge ClosestEdge(const List<Vector2>& simplex);
	static Vector2 Support(const Vector<Vector2>& shape0, const Vector<Vector2>& shape1, const Vector2& direction);
	static Vector2 TripleProduct(const Vector2& a, const Vector2& b, const Vector2& c);

	static List<PhysicsObject2D*> physicsObjects2D;
	static List<PhysicsObject3D*> physicsObjects3D;
	
	static Array<Array<Collision2DFn, COLLIDER_2D_MAX>, COLLIDER_2D_MAX> collision2DTable;

	static class ContactManager* contactManager;

	static F64 airDensity;
	static F64 gravity;
	static bool newContacts;

	friend class Engine;
};