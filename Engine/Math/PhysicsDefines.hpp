/*
* This is a modified version of Box2D for C++: https://github.com/eXpl0it3r/Box2D
*
* Copyright (c) 2011 Erin Catto http://box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include "Resources\Component.hpp"
#include "Math\Math.hpp"

enum NH_API PhysicsEventType
{
	PHYSICS_EVENT_ON_COLLISION,
	PHYSICS_EVENT_ON_TRIGGER_ENTER,
	PHYSICS_EVENT_ON_TRIGGER_EXIT,
};

enum NH_API ColliderType
{
	COLLIDER_TYPE_POLYGON,
	COLLIDER_TYPE_CIRCLE,
};

static constexpr inline U8 MaxPolygonVertices = 8;
static constexpr inline U8 MaxManifoldPoints = 2;
static constexpr inline F32 VelocityThreshold = 1.0f;
static constexpr inline U32 VelocityIterations = 6;
static constexpr inline U32 PositionIterations = 2;
static constexpr inline F32 MaxLinearCorrection = 0.2f;
static constexpr inline F32 MaxAngularCorrection = (8.0f / 180.0f * PI_F);
static constexpr inline F32 MaxTranslation = 2.0f;
static constexpr inline F32 MaxTranslationSquared = (MaxTranslation * MaxTranslation);
static constexpr inline F32 MaxRotation = (0.5f * PI_F);
static constexpr inline F32 MaxRotationSquared = (MaxRotation * MaxRotation);
static constexpr inline F32 Baumgarte = 0.2f;
static constexpr inline F32 TOIBaugarte = 0.75f;
static constexpr inline F32 AllowedPenetration = 0.01f;
static constexpr inline F32 LinearSlop = 0.005f;
static constexpr inline F32 AngularSlop = 2.0f / 180.0f * PI_F;
static constexpr inline F32 TimeToSleep = 0.5f;
static constexpr inline F32 LinearSleepTolerance = 0.01f;
static constexpr inline F32 LinearSleepToleranceSqr = LinearSleepTolerance * LinearSleepTolerance;
static constexpr inline F32 AngularSleepTolerance = (2.0f / 180.0f * PI_F);
static constexpr inline F32 AngularSleepToleranceSqr = AngularSleepTolerance * AngularSleepTolerance;
static constexpr inline F32 AABBExtension = 0.1f;
static constexpr inline F32 AABBMultiplier = 2.0f;
static constexpr inline U32 MaxTOIContacts = 32;
static constexpr inline U32 MaxSubSteps = 8;

struct NH_API Transform2D
{
	Vector2 position{ Vector2Zero };
	Quaternion2 rotation{ Quaternion2Identity };

	constexpr Transform2D operator*(const Transform2D& t) const
	{
		return { t.position * rotation + position, rotation * t.rotation };
	}

	constexpr Transform2D operator^(const Transform2D& t) const
	{
		return { (t.position - position) ^ rotation, rotation ^ t.rotation };
	}
};

static constexpr Vector2 operator*(const Vector2& v, const Transform2D& t) { return v * t.rotation + t.position; }

static constexpr Vector2 operator^(const Vector2& v, const Transform2D& t) { return (v ^ t.rotation) + t.position; }

struct ContactFeature
{
	enum Type
	{
		Vertex = 0,
		Face = 1
	};

	U8 indexA;
	U8 indexB;
	U8 typeA;
	U8 typeB;
};

union ContactID
{
	ContactFeature cf;
	U32 key;
};

struct ManifoldPoint
{
	Vector2 localPoint;
	F32 normalImpulse;
	F32 tangentImpulse;
	ContactID id;
};

struct Manifold2D
{
	enum Type
	{
		Circles,
		FaceA,
		FaceB
	};

	ManifoldPoint points[MaxManifoldPoints];
	Vector2 localNormal;
	Vector2 localPoint;
	Type type;
	U32 pointCount;
};

struct Sweep2D
{
	Transform2D GetTransform(F32 beta) const
	{
		Transform2D transform{};
		transform.position = Math::Lerp(c0, c, beta);
		transform.rotation = Math::Lerp(a0, a, beta);

		transform.position -= localCenter * transform.rotation;

		return transform;
	}

	void Advance(F32 alpha)
	{
		F32 beta = (alpha - alpha0) / (1.0f - alpha0);
		c0 += beta * (c - c0);
		a0 += beta * (a - a0);
		alpha0 = alpha;
	}

	void Normalize()
	{
		F32 d = TWO_PI_F * Math::FloorF(a0 / TWO_PI_F);
		a0 -= d;
		a -= d;
	}

	Vector2 localCenter;
	Vector2 c0, c;
	F32 a0, a;

	F32 alpha0;
};

struct AABB
{
	void Combine(const AABB& a, const AABB& b)
	{
		lowerBound = Math::Min(a.lowerBound, b.lowerBound);
		upperBound = Math::Max(a.upperBound, b.upperBound);
	}

	void Combine(const AABB& other)
	{
		lowerBound = Math::Min(lowerBound, other.lowerBound);
		upperBound = Math::Max(upperBound, other.upperBound);
	}

	F32 GetPerimeter() const
	{
		F32 wx = upperBound.x - lowerBound.x;
		F32 wy = upperBound.y - lowerBound.y;
		return (wx + wy) * 2.0f;
	}

	bool Contains(const AABB& aabb) const
	{
		bool result = true;
		result = result && lowerBound.x <= aabb.lowerBound.x;
		result = result && lowerBound.y <= aabb.lowerBound.y;
		result = result && aabb.upperBound.x <= upperBound.x;
		result = result && aabb.upperBound.y <= upperBound.y;
		return result;
	}

	Vector2 lowerBound;
	Vector2 upperBound;
};

struct NH_API ColliderInfo
{
	bool trigger = false;
	U64 layers = 1;
	F32 restitution = 0.0f;
	F32 staticFriction = 0.2f;
	F32 dynamicFriction = 0.2f;
	F32 density = 1.0f;
	Vector2 center = Vector2Zero;
};

struct MassData
{
	Vector2 center;
	F32 mass;
	F32 I;
};

struct RigidBody2D;
struct Collider2D;

struct RigidBody2DRef
{
	RigidBody2DRef() {}
	RigidBody2DRef(U64 index) : index(index) {}

	RigidBody2D& operator*();
	RigidBody2D* operator->();
	RigidBody2D* Data();

	bool operator==(const RigidBody2D* other) const;
	bool operator==(const RigidBody2D& other) const;

private:
	U64 index = U64_MAX;
};

struct Collider2DRef
{
	Collider2DRef() {}
	Collider2DRef(U64 index, RigidBody2DRef body) : index(index), body(body) {}
	Collider2DRef(U64 colliderIndex, U64 bodyIndex) : index(colliderIndex), body({ bodyIndex }) {}

	Collider2D& operator*();
	Collider2D* operator->();
	Collider2D* Data();

private:
	RigidBody2DRef body = { U64_MAX };
	U64 index = U64_MAX;
};

struct ColliderProxy
{
	AABB aabb;
	Collider2DRef collider;
	U32 proxyId;
};

struct NH_API Collider2D
{
	ColliderType type;

	struct Polygon
	{
		U8 vertexCount;
		Vector2 vertices[MaxPolygonVertices];
		Vector2 normals[MaxPolygonVertices];
	};

	AABB ComputeAABB(const Transform2D& transform)
	{
		switch (type)
		{
		case COLLIDER_TYPE_POLYGON: {
			Vector2 lower = (polygon.vertices[0] + center) * transform;
			Vector2 upper = lower;

			for (U32 i = 1; i < polygon.vertexCount; ++i)
			{
				Vector2 v = (polygon.vertices[i] + center) * transform;
				lower = { Math::Min(lower.x, v.x), Math::Min(lower.y, v.y) };
				upper = { Math::Max(upper.x, v.x), Math::Max(upper.y, v.y) };
			}

			Vector2 r{ radius, radius };

			return { lower - r, upper + r };
		} break;
		case COLLIDER_TYPE_CIRCLE: {
			Vector2 p = transform.position + center * transform.rotation;

			return { {p.x - radius, p.y - radius}, {p.x + radius, p.y + radius} };
		} break;
		default: return {};
		}
	}

	MassData ComputeMass()
	{
		switch (type)
		{
		case COLLIDER_TYPE_POLYGON: {
			Vector2 com = Vector2Zero;
			F32 area = 0.0f;
			F32 I = 0.0f;

			// s is the reference point for forming triangles.
			// It's location doesn't change the result (except for rounding error).
			Vector2 s = Vector2Zero;

			// This code would put the reference point inside the polygon.
			for (U32 i = 0; i < polygon.vertexCount; ++i)
			{
				s += polygon.vertices[i];
			}
			s *= 1.0f / polygon.vertexCount;

			const F32 k_inv3 = 1.0f / 3.0f;

			Vector2 prev = polygon.vertices[polygon.vertexCount - 1];

			for (U32 i = 0; i < (U32)(polygon.vertexCount - 1); ++i)
			{
				// Triangle vertices.
				Vector2 e1 = prev - s;
				Vector2 e2 = polygon.vertices[i] - s;

				F32 D = e1.Cross(e2);

				F32 triangleArea = 0.5f * D;
				area += triangleArea;

				// Area weighted centroid
				com += triangleArea * k_inv3 * (e1 + e2);

				F32 ex1 = e1.x, ey1 = e1.y;
				F32 ex2 = e2.x, ey2 = e2.y;

				F32 intx2 = ex1 * ex1 + ex2 * ex1 + ex2 * ex2;
				F32 inty2 = ey1 * ey1 + ey2 * ey1 + ey2 * ey2;

				I += (0.25f * k_inv3 * D) * (intx2 + inty2);

				prev = e2;
			}

			Vector2 massCenter = com + s;
			F32 mass = density * area;

			return { massCenter, density * area, density * I + mass * (massCenter.Dot(massCenter) - com.Dot(com)) };
		} break;
		case COLLIDER_TYPE_CIRCLE: {
			F32 mass = density * PI_F * radius * radius;
			F32 I = mass * (0.5f * radius * radius * center.Dot(center));

			return { center, mass, I };
		} break;
		default: return {};
		}
	}

	//TODO: Full event system
	//PhysicsEvent2D event{ nullptr };

	bool trigger = false;
	U64 layers = 1;
	F32 restitution = 0.0f;
	F32 staticFriction = 0.2f;
	F32 dynamicFriction = 0.2f;
	F32 density = 1.0f;
	F32 radius = 1.0f;
	Vector2 center = Vector2Zero;
	Polygon polygon;

	ColliderProxy proxy;

	RigidBody2DRef body;
};

struct Contact2D
{
private:
	enum Flags
	{
		//Flags
		FLAG_ISLAND = 0x0001,
		FLAG_TOUCHING = 0x0002,
		FLAG_ENABLED = 0x0004,
		FLAG_FILTER = 0x0008,
		FLAG_BULLET_HIT = 0x0010,
		FLAG_TOI = 0x0020,

		//Masks
		MASK_TOUCHING_ENABLED = FLAG_TOUCHING | FLAG_ENABLED,
	};

	void Update();

	Manifold2D manifold;

	Collider2DRef colliderA;
	Collider2DRef colliderB;

	I32 toiCount;
	F32 toi;

	F32 friction;
	F32 restitution;

	F32 tangentSpeed;

	U8 flags;
	bool valid;
	U32 index;

	friend class Physics;
	friend struct Island2D;
	friend struct RigidBody2D;
};

struct ContactEdge2D
{
	RigidBody2DRef other;
	Contact2D* contact;
};

enum NH_API BodyType
{
	BODY_TYPE_DYNAMIC,
	BODY_TYPE_KINEMATIC,
	BODY_TYPE_STATIC,
};

struct NH_API RigidBody2D : public Component
{
	RigidBody2D(BodyType type) : type(type), flags(FLAG_ACTIVE | FLAG_AWAKE | FLAG_AUTO_SLEEP)
	{
		if (type == BODY_TYPE_DYNAMIC) { invMass = 1.0f; }
		else { invMass = 0.0f; }
	}
	RigidBody2D(RigidBody2D&& other) noexcept : Component(Move(other)), type(other.type), colliders(Move(other.colliders)), transform(other.transform), sweep(other.sweep),
		velocity(other.velocity), force(other.force), angularVelocity(other.angularVelocity), torque(other.torque), invMass(other.invMass), invInertia(other.invInertia),
		linearDrag(other.linearDrag), angularDrag(other.angularDrag), gravityScale(other.gravityScale), flags(other.flags), sleepTime(other.sleepTime)
	{
	}

	RigidBody2D& operator=(RigidBody2D&& other) noexcept
	{
		Component::operator=(Move(other));
		type = other.type;
		colliders = Move(other.colliders);
		transform = other.transform;
		sweep = other.sweep;
		velocity = other.velocity;
		force = other.force;
		angularVelocity = other.angularVelocity;
		torque = other.torque;
		invMass = other.invMass;
		invInertia = other.invInertia;
		linearDrag = other.linearDrag;
		angularDrag = other.angularDrag;
		gravityScale = other.gravityScale;
		flags = other.flags;
		sleepTime = other.sleepTime;

		return *this;
	}

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;
	virtual void Cleanup(Scene* scene) final;

	void AddCollider(const Collider2D& collider);

	const Transform2D& Transform() const { return transform; }
	const Vector2& Position() const { return transform.position; }
	const Vector2& Velocity() const { return velocity; }
	const Vector2& Force() const { return force; }
	const Quaternion2& Rotation() const { return transform.rotation; }
	F32 AngularVelocity() const { return angularVelocity; }
	F32 Torque() const { return torque; }

	void SetPosition(const Vector2& newPosition) { transform.position = newPosition; UpdateTransform(); }
	void SetRotation(const Quaternion2& newRotation) { transform.rotation = newRotation; UpdateTransform(); }
	void SetTransform(const Transform2D& newTransform) { transform = newTransform; UpdateTransform(); }
	void SetVelocity(const Vector2& newVelocity)
	{
		if (type == BODY_TYPE_STATIC) { return; }

		if (newVelocity.Dot(newVelocity) > 0.0f) { SetAwake(true); }

		velocity = newVelocity;
	}
	void SetAngularVelocity(F32 newAngularVelocity)
	{
		if (type == BODY_TYPE_STATIC) { return; }

		if (newAngularVelocity * newAngularVelocity > 0.0f) { SetAwake(true); }

		angularVelocity = newAngularVelocity;
	}

	void ApplyForce(const Vector2& force, const Vector2& point)
	{
		if (type != BODY_TYPE_DYNAMIC) { return; }

		if ((flags & FLAG_AWAKE) == 0) { SetAwake(true); }

		this->force += force;
		torque += (point - sweep.c).Cross(force);
	}

	void ApplyForceToCenter(const Vector2& force)
	{
		if (type != BODY_TYPE_DYNAMIC) { return; }

		if ((flags & FLAG_AWAKE) == 0) { SetAwake(true); }

		this->force += force;
	}

	void ApplyTorque(F32 torque)
	{
		if (type != BODY_TYPE_DYNAMIC) { return; }

		if ((flags & FLAG_AWAKE) == 0) { SetAwake(true); }

		this->torque += torque;
	}

	void ApplyImpulse(const Vector2& impulse, const Vector2& point)
	{
		if (type != BODY_TYPE_DYNAMIC) { return; }

		if ((flags & FLAG_AWAKE) == 0) { SetAwake(true); }

		velocity += impulse * invMass;
		angularVelocity += (point - sweep.c).Cross(impulse) * invInertia;
	}

	void ApplyImpulseToCenter(const Vector2& impulse)
	{
		if (type != BODY_TYPE_DYNAMIC) { return; }

		if ((flags & FLAG_AWAKE) == 0) { SetAwake(true); }

		velocity += impulse * invMass;
	}

	void ApplyAngularImpulse(F32 impulse)
	{
		if (type != BODY_TYPE_DYNAMIC) { return; }

		if ((flags & FLAG_AWAKE) == 0) { SetAwake(true); }

		angularVelocity += impulse * invInertia;
	}

	void SetMass(F32 mass) { if (mass <= 0.0f) { invMass = 0.0f; } else { invMass = 1.0f / mass; } }
	void SetBodyType(BodyType type);

	void SetAwake(bool b)
	{
		if (b)
		{
			if ((flags & FLAG_AWAKE) == 0)
			{
				flags |= FLAG_AWAKE;
				sleepTime = 0.0f;
			}
		}
		else
		{
			flags &= ~FLAG_AWAKE;
			sleepTime = 0.0f;
			velocity = Vector2Zero;
			angularVelocity = 0.0f;
			force = Vector2Zero;
			torque = 0.0f;
		}
	}

	void SetActive(bool b);

private:
	enum Flags
	{
		//Flags
		FLAG_ISLAND = 0x0001,
		FLAG_AWAKE = 0x0002,
		FLAG_AUTO_SLEEP = 0x0004,
		FLAG_BULLET = 0x0008,
		FLAG_FIXED_ROTATION = 0x0010,
		FLAG_ACTIVE = 0x0020,
		FLAG_TOI = 0x0040,

		//Masks
		MASK_AWAKE_ACTIVE = FLAG_AWAKE | FLAG_ACTIVE,
	};

	bool ShouldCollide(RigidBody2D* rb)
	{
		return type == BODY_TYPE_DYNAMIC || rb->type == BODY_TYPE_DYNAMIC;
	}

	void SynchronizeTransform()
	{
		transform.rotation = sweep.a;
		transform.position = sweep.c - sweep.localCenter * transform.rotation;
	}

	void Advance(F32 alpha)
	{
		// Advance to the new safe time. This doesn't sync the broad-phase.
		sweep.Advance(alpha);
		sweep.c = sweep.c0;
		sweep.a = sweep.a0;
		SynchronizeTransform();
	}

	void UpdateTransform();
	void SynchronizeFixtures();

	void ResetMassData()
	{
		// Compute mass data from shapes. Each shape has its own density.
		F32 mass = 0.0f;
		F32 I = 0.0f;
		invMass = 0.0f;
		invInertia = 0.0f;
		sweep.localCenter = Vector2Zero;

		if (type == BODY_TYPE_STATIC || type == BODY_TYPE_KINEMATIC)
		{
			sweep.c0 = transform.position;
			sweep.c = transform.position;
			sweep.a0 = sweep.a;
			return;
		}

		// Accumulate mass over all fixtures.
		Vector2 localCenter = Vector2Zero;
		for (Collider2D& collider : colliders)
		{
			MassData massData = collider.ComputeMass();
			mass += massData.mass;
			localCenter += massData.mass * massData.center;
			I += massData.I;
		}

		// Compute center of mass.
		if (mass > 0.0f)
		{
			invMass = 1.0f / mass;
			localCenter *= invMass;
		}
		else
		{
			// Force all dynamic bodies to have a positive mass.
			mass = 1.0f;
			invMass = 1.0f;
		}

		if (I > 0.0f && (flags & FLAG_FIXED_ROTATION) == 0)
		{
			// Center the inertia about the center of mass.
			I -= mass * localCenter.Dot(localCenter);
			invInertia = 1.0f / I;

		}
		else
		{
			I = 0.0f;
			invInertia = 0.0f;
		}

		// Move center of mass.
		Vector2 oldCenter = sweep.c;
		sweep.localCenter = localCenter;
		sweep.c0 = sweep.c = sweep.localCenter * transform;

		// Update center of mass velocity.
		velocity += (sweep.c - oldCenter).CrossInv(angularVelocity);
	}

	BodyType type{ BODY_TYPE_DYNAMIC };

	U64 index;
	Vector<Collider2D> colliders;
	Vector<ContactEdge2D> contacts;
	Transform2D transform;
	Sweep2D sweep;

	Vector2 velocity = Vector2Zero;
	Vector2 force = Vector2Zero;

	F32 angularVelocity = 0.0f;
	F32 torque = 0.0f;

	F32 invMass = 1.0f;
	F32 invInertia = 0.0f;
	F32 linearDrag = 0.0f;
	F32 angularDrag = 0.0f;
	F32 gravityScale = 1.0f;

	U8 flags = 0;
	F32 sleepTime = 0.0f;
	U32 islandIndex = U32_MAX;

	friend class Physics;
	friend class Broadphase; //TODO: temp
	friend struct Island2D;
	friend struct Collider2DRef;
};

struct DistanceProxy
{
	DistanceProxy() : vertices(NULL), count(0), radius(0.0f) {}

	void Set(const Collider2D* shape)
	{
		switch (shape->type)
		{
		case COLLIDER_TYPE_CIRCLE: {
			vertices = &shape->center;
			count = 1;
			radius = shape->radius;
		} break;
		case COLLIDER_TYPE_POLYGON: {
			vertices = shape->polygon.vertices;
			count = shape->polygon.vertexCount;
			radius = shape->radius;
		} break;
		}
	}
	U32 GetSupport(const Vector2& d) const
	{
		U32 bestIndex = 0;
		F32 bestValue = vertices[0].Dot(d);
		for (U32 i = 1; i < count; ++i)
		{
			F32 value = vertices[i].Dot(d);
			if (value > bestValue)
			{
				bestIndex = i;
				bestValue = value;
			}
		}

		return bestIndex;
	}
	const Vector2& GetSupportVertex(const Vector2& d) const
	{
		U32 bestIndex = 0;
		F32 bestValue = vertices[0].Dot(d);
		for (U32 i = 1; i < count; ++i)
		{
			F32 value = vertices[i].Dot(d);
			if (value > bestValue)
			{
				bestIndex = i;
				bestValue = value;
			}
		}

		return vertices[bestIndex];
	}

	Vector2 buffer[2];
	const Vector2* vertices;
	U32 count;
	F32 radius;
};

struct TOIInput
{
	DistanceProxy proxyA;
	DistanceProxy proxyB;
	Sweep2D sweepA;
	Sweep2D sweepB;
	F32 tMax;
};

struct TOIOutput
{
	enum State
	{
		State_Unknown,
		State_Failed,
		State_Overlapped,
		State_Touching,
		State_Separated
	};

	State state;
	F32 t;
};

struct SimplexCache
{
	F32 metric;
	U16 count;
	U8 indexA[3];
	U8 indexB[3];
};

struct DistanceInput
{
	DistanceProxy proxyA;
	DistanceProxy proxyB;
	Transform2D transformA;
	Transform2D transformB;
	bool useRadii;
};

struct DistanceOutput
{
	Vector2 pointA;
	Vector2 pointB;
	F32 distance;
	U32 iterations;
};

struct SimplexVertex
{
	Vector2 wA;
	Vector2 wB;
	Vector2 w;
	F32 a;
	U32 indexA;
	U32 indexB;
};

struct Simplex
{
	void ReadCache(const SimplexCache* cache,
		const DistanceProxy* proxyA, const Transform2D& transformA,
		const DistanceProxy* proxyB, const Transform2D& transformB);

	void WriteCache(SimplexCache* cache) const;
	Vector2 GetSearchDirection() const;
	Vector2 GetClosestPoint() const;
	void GetWitnessPoints(Vector2* pA, Vector2* pB) const;
	F32 GetMetric() const;
	void Solve2();
	void Solve3();

	SimplexVertex v1, v2, v3;
	U32 count;
};

struct SeparationFunction
{
	enum Type
	{
		Points,
		FaceA,
		FaceB
	};

	F32 Initialize(const SimplexCache* cache,
		const DistanceProxy* proxyA, const Sweep2D& sweepA,
		const DistanceProxy* proxyB, const Sweep2D& sweepB,
		F32 t1);
	F32 FindMinSeparation(U32* indexA, U32* indexB, F32 t) const;
	F32 Evaluate(U32 indexA, U32 indexB, F32 t) const;

	const DistanceProxy* proxyA;
	const DistanceProxy* proxyB;
	Sweep2D sweepA, sweepB;
	Type type;
	Vector2 localPoint;
	Vector2 axis;
};

struct ClipVertex
{
	Vector2 v;
	ContactID id;
};