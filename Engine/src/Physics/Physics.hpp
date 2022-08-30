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
	BOX_COLLIDER,
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

	Vector2 Size() const
	{
		return Vector2(xBounds.y - xBounds.x, yBounds.y - yBounds.x);
	}

	bool Contains(const Box& b) const
	{
		return b.xBounds.x <= xBounds.y && b.xBounds.y >= xBounds.x && b.yBounds.x <= yBounds.y && b.yBounds.y >= yBounds.x;
	}

	bool Contains(const Vector2& point)
	{
		return point.x >= xBounds.x && point.x <= xBounds.y && point.y >= yBounds.x && point.y <= yBounds.y;
	}

	F32 Perimeter() const
	{
		F32 wx = xBounds.y - xBounds.x;
		F32 wy = yBounds.y - yBounds.x;
		return 2.0f * (wx + wy);
	}

	F32 Area() const
	{
		F32 wx = xBounds.y - xBounds.x;
		F32 wy = yBounds.y - yBounds.x;
		return wx * wy;
	}

	void Merge(const Box& box)
	{
		xBounds.x = Math::Min(xBounds.x, box.xBounds.x);
		xBounds.y = Math::Max(xBounds.y, box.xBounds.y);
		yBounds.x = Math::Min(yBounds.x, box.xBounds.x);
		yBounds.y = Math::Max(yBounds.y, box.xBounds.y);
	}

	Box Merged(const Box& box) const
	{
		return { {Math::Min(xBounds.x, box.xBounds.x), Math::Max(xBounds.y, box.xBounds.y)},
			{Math::Min(yBounds.x, box.xBounds.x), Math::Max(yBounds.y, box.xBounds.y)} };
	}

	void Merge(const Box& box0, const Box& box1)
	{
		xBounds.x = Math::Min(box0.xBounds.x, box1.xBounds.x);
		xBounds.y = Math::Max(box0.xBounds.y, box1.xBounds.y);
		yBounds.x = Math::Min(box0.yBounds.x, box1.xBounds.x);
		yBounds.y = Math::Max(box0.yBounds.y, box1.xBounds.y);
	}

	Box Merged(const Box& box0, const Box& box1) const
	{
		return { {Math::Min(box0.xBounds.x, box1.xBounds.x), Math::Max(box0.xBounds.y, box1.xBounds.y)},
			{Math::Min(box0.yBounds.x, box1.xBounds.x), Math::Max(box0.yBounds.y, box1.xBounds.y)} };
	}

	Box Minkowski(const Box& b)
	{
		return { {xBounds.x - b.xBounds.y, xBounds.y - b.xBounds.x}, {yBounds.x - b.yBounds.y, yBounds.y - b.yBounds.x} };
	}

	Vector2 DirectionToClosestBound(const Vector2& point)
	{
		F32 minDist = Math::Abs(point.x - xBounds.x);
		Vector2 closest{ xBounds.x, point.y };

		if (Math::Abs(xBounds.y - point.x) < minDist)
		{
			minDist = Math::Abs(xBounds.y - point.x);
			closest = { xBounds.y, point.x };
		}
		if (Math::Abs(yBounds.y - point.y) < minDist)
		{
			minDist = Math::Abs(yBounds.y - point.y);
			closest = { point.x, yBounds.y };
		}
		if (Math::Abs(yBounds.x - point.y) < minDist)
		{
			minDist = Math::Abs(yBounds.x - point.y);
			closest = { point.x, yBounds.x };
		}

		return closest;
	}

	F32 TOI(const Vector2& origin, const Vector2& direction) const;

	Box operator+(const Vector2& v) const { return{ xBounds + v.x, yBounds + v.y }; }

	Box Fattened(F32 f) const 
	{ 
		F32 x = (xBounds.y - xBounds.x) * f; 
		F32 y = (yBounds.y - yBounds.x) * f; 
		return { {xBounds.x - x, xBounds.y + x}, {yBounds.x - y, yBounds.y + y} }; 
	}

	Box& Fatten(F32 f)
	{
		F32 x = (xBounds.y - xBounds.x) * f;
		F32 y = (yBounds.y - yBounds.x) * f;
		xBounds = { xBounds.x - x, xBounds.y + x };
		yBounds = { yBounds.x - y, yBounds.y + y };
		return *this;
	}
};

struct Shape
{
	Vector<Vector2> vertices;
	Vector<Vector2> normals;
};

struct Collider2D
{
	Collider2DType type;

	bool trigger;

	Box box;
};

struct BoxCollider : public Collider2D
{

};

struct PolygonCollider : public Collider2D
{
	Shape shape;
};

struct CircleCollider : public Collider2D
{
	Vector2 offset;
	F64 radius;
};

struct PhysicsObject2DConfig
{
	Collider2DType type;

	//Circle
	F64 radius;
	Vector2 offset;

	//Polygon
	Vector<Vector2> shape;

	//Box
	Box box;

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

struct Support
{
	Vector2 sA;
	Vector2 sB;
	Vector2 p;
	I32 iA;
	I32 iB;
	F32 u{ 0.0f };
};

struct Simplex
{
	Support a;
	Support b;
	Support c;
	Support d;
	I32 count;
	F32 div;
};

struct HalfSpace
{
	Vector2 normal;
	F32 distance;

	F32 Distance(const Vector2& v) const { return normal.Dot(v) - distance; }
};

struct GJKCache
{
	F32 metric;
	I32 count;
	I32 iA[3];
	I32 iB[3];
	F32 div;
};

struct Edge
{
	Vector2 vertex0;
	Vector2 vertex1;
	Vector2 normal;
	F32 distance;
	U32 index;
};

struct Contact2D
{
	struct PhysicsObject2D* a;
	struct PhysicsObject2D* b;

	F32 restitution;
	F32 penetration;
	Vector2 relativeVelocity;
	Vector2 normal;
};

struct PhysicsObject2D
{
	void ApplyForce(const Vector2& f) { force += f; }
	void ApplyTorque(F64 t) { torque += t; }
	void AddVelocity(const Vector2& v) { velocity += v; }
	void Translate(const Vector2& v) { oneTimeVelocity += v; }
	void AddAngularVelocity(F64 v) { angularVelocity += v; }
	void SetGravityScale(F64 s) { gravityScale = s; }

private:
	U64 id;
	U32 proxyID;
	Collider2D* collider;
	Transform2D* transform;

	//secondary
	Vector2 velocity;
	Vector2 oneTimeVelocity;
	Vector2 move;
	F32 angularVelocity;

	// secondary
	Vector2 force;
	F32 torque;

	bool grounded;
	bool stopped;
	Vector2 axisLock;
		
	// constants
	F32 mass;
	F32 massInv;
	F32 inertia;
	F32 inertiaInv;
	F32 friction;
	F32 restitution;
	F32 gravityScale;
	F32 dragCoefficient;
	F32 angularDragCoefficient;
	F32 area;
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
	friend struct BoxTree;
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
	static F32 TOI(const Vector2& p, const Vector2& endP, const Vector2& q, const Vector2& endQ);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update(F64 step);

	static void BroadPhase();
	static void NarrowPhase();
	static bool BoxVsBox(Contact2D& c);
	static bool CircleVsCircle(Contact2D& c);
	static bool PolygonVsPolygon(Contact2D& c);
	static bool PolygonVsCircle(Contact2D& c);
	static bool CircleVsPolygon(Contact2D& c);
	static void ResolveCollision(Contact2D& c);

	static bool ContainsOrigin(List<Vector2>& simplex, Vector2& direction);
	static Vector2 FarthestPoint(const Vector<Vector2>& shape, const Vector2& direction);
	static Edge ClosestEdge(const List<Vector2>& simplex);
	static Vector2 FindSupport(const Vector<Vector2>& shape0, const Vector<Vector2>& shape1, const Vector2& direction);
	static Vector2 TripleProduct(const Vector2& a, const Vector2& b, const Vector2& c);

	static List<PhysicsObject2D*> physicsObjects2D;
	static List<PhysicsObject3D*> physicsObjects3D;

	static Array<Array<Collision2DFn, COLLIDER_2D_MAX>, COLLIDER_2D_MAX> collision2DTable;

	static struct BoxTree* tree;
	//static class ContactManager* contactManager;
	static struct BoolTable table; //TODO: temp

	static F64 airDensity;
	static F64 gravity;
	static bool newContacts;

	friend class Engine;
};