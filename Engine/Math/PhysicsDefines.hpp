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
#include "SIMD.hpp"

import ThreadSafety;

#define PROXY_TYPE(KEY) ((BodyType)((KEY) & 3))
#define PROXY_ID(KEY) ((KEY) >> 2)
#define PROXY_KEY(ID, TYPE) (((ID) << 2) | (TYPE))
#define SHAPE_PAIR_KEY(K1, K2) K1 < K2 ? (U64)K1 << 32 | (U64)K2 : (U64)K2 << 32 | (U64)K1

static constexpr inline I32 NullIndex = -1;
static constexpr inline U8 MaxPolygonVertices = 8;
static constexpr inline U8 MaxManifoldPoints = 2;
static constexpr inline F32 VelocityThreshold = 1.0f;
static constexpr inline U32 VelocityIterations = 6;
static constexpr inline U32 PositionIterations = 2;
static constexpr inline F32 MaxLinearCorrection = 0.2f;
static constexpr inline F32 MaxAngularCorrection = (8.0f / 180.0f * PI_F);
static constexpr inline F32 MaxTranslation = 2.0f;
static constexpr inline F32 MaxTranslationSquared = (MaxTranslation * MaxTranslation);
static constexpr inline F32 MaxRotation = (0.25f * PI_F);
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
static constexpr inline F32 AABBMargin = 0.1f;
static constexpr inline U32 MaxTOIContacts = 32;
static constexpr inline U32 MaxSubSteps = 8;
static constexpr inline U32 MaxWorkers = 64;
static constexpr inline U32 GraphColorCount = 12;
static constexpr inline U32 OverflowIndex = GraphColorCount - 1;
static constexpr inline F32 SpeculativeDistance = 4.0f * LinearSlop;

enum NH_API ColliderType
{
	COLLIDER_TYPE_POLYGON,
	COLLIDER_TYPE_CIRCLE,

	COLLIDER_TYPE_COUNT
};

enum NH_API ShapeType
{
	SHAPE_TYPE_CIRCLE,
	SHAPE_TYPE_CAPSULE,
	SHAPE_TYPE_SEGMENT,
	SHAPE_TYPE_POLYGON,
	SHAPE_TYPE_CHAIN_SEGMENT,

	SHAPE_TYPE_COUNT
};

enum NH_API JointType
{
	JOINT_TYPE_DISTANCE,
	JOINT_TYPE_MOTOR,
	JOINT_TYPE_MOUSE,
	JOINT_TYPE_PRISMATIC,
	JOINT_TYPE_REVOLUTE,
	JOINT_TYPE_WELD,
	JOINT_TYPE_WHEEL,

	JOINT_TYPE_COUNT
};

enum NH_API BodyType
{
	BODY_TYPE_STATIC,
	BODY_TYPE_KINEMATIC,
	BODY_TYPE_DYNAMIC,

	BODY_TYPE_COUNT
};

enum TOIState
{
	TOI_STATE_UNKNOWN,
	TOI_STATE_FAILED,
	TOI_STATE_OVERLAPPED,
	TOI_STATE_HIT,
	TOI_STATE_SEPARATED,

	TOI_STATE_COUNT
};

enum ContactFlag
{
	CONTACT_FLAG_TOUCHING = 0x00000001,
	CONTACT_FLAG_HIT_EVENT = 0x00000002,
	CONTACT_FLAG_SENSOR = 0x0000004,
	CONTACT_FLAG_SENSOR_TOUCHING = 0x00000008,
	CONTACT_FLAG_ENABLE_SENSOR_EVENTS = 0x00000010,
	CONTACT_FLAG_ENABLE_CONTACT_EVENTS = 0x00000020,
};

enum ContactSimFlag
{
	CONTACT_SIM_FLAG_TOUCHING = 0x00010000,
	CONTACT_SIM_FLAG_DISJOINT = 0x00020000,
	CONTACT_SIM_FLAG_STARTED_TOUCHING = 0x00040000,
	CONTACT_SIM_FLAG_STOPPED_TOUCHING = 0x00080000,
	CONTACT_SIM_FLAG_ENABLE_HIT_EVENT = 0x00100000,
	CONTACT_SIM_FLAG_ENABLE_PRESOLVE_EVENTS = 0x00200000,
};

enum SolverStageType
{
	SOLVER_STAGE_PREPARE_JOINTS,
	SOLVER_STAGE_PREPARE_CONTACTS,
	SOLVER_STAGE_INTEGRATE_VELOCITIES,
	SOLVER_STAGE_WARM_START,
	SOLVER_STAGE_SOLVE,
	SOLVER_STAGE_INTEGRATE_POSITIONS,
	SOLVER_STAGE_RELAX,
	SOLVER_STAGE_RESTITUTION,
	SOLVER_STAGE_STORE_IMPULSES
};

enum SolverBlockType
{
	SOLVER_BLOCK_TYPE_BODY,
	SOLVER_BLOCK_TYPE_JOINT,
	SOLVER_BLOCK_TYPE_CONTACT,
	SOLVER_BLOCK_TYPE_GRAPH_JOINT,
	SOLVER_BLOCK_TYPE_GRAPH_CONTACT
};

enum SetType
{
	SET_TYPE_STATIC = 0,
	SET_TYPE_DISABLED = 1,
	SET_TYPE_AWAKE = 2,
	SET_TYPE_FIRST_SLEEPING = 3
};

enum SeparationType
{
	SEPARATION_TYPE_POINTS,
	SEPARATION_TYPE_FACE_A,
	SEPARATION_TYPE_FACE_B
};

struct NH_API RaycastInput
{
	bool Valid()
	{
		return origin.Valid() && translation.Valid() && Math::IsValid(maxFraction) && maxFraction >= 0.0f && maxFraction < 100000.0f;
	}

	Vector2 origin;
	Vector2 translation;
	F32 maxFraction;
};

struct ShapeCastInput
{
	Vector2 points[MaxPolygonVertices];
	I32 count;
	F32 radius;
	Vector2 translation;
	F32 maxFraction;
};

struct CastOutput
{
	Vector2 normal;
	Vector2 point;
	F32 fraction;
	I32 iterations;
	bool hit;
};

struct MassData
{
	F32 mass;
	Vector2 center;
	F32 rotationalInertia;
};

struct Circle
{
	Vector2 center;
	F32 radius;
};

struct Capsule
{
	Vector2 center1;
	Vector2 center2;
	F32 radius;
};

struct Polygon
{
	Vector2 vertices[MaxPolygonVertices];
	Vector2 normals[MaxPolygonVertices];
	Vector2 centroid;
	F32 radius;
	I32 count;
};

struct Segment
{
	Vector2 point1;
	Vector2 point2;
};

struct ChainSegment
{
	Vector2 ghost1;
	Segment segment;
	Vector2 ghost2;
	I32 chainId;
};

struct Hull
{
	Vector2 points[MaxPolygonVertices];
	I32 count;
};

struct Filter
{
	bool ShouldShapesCollide(const Filter& other);

	U64 layers;
	U64 layerMask;
	I32 groupIndex;
};

struct AABB
{
	static AABB Combine(const AABB& a, const AABB& b)
	{
		return { Math::Min(a.lowerBound, b.lowerBound), Math::Max(a.upperBound, b.upperBound) };
	}

	static bool Enlarge(AABB& a, const AABB& b)
	{
		bool changed = false;
		if (b.lowerBound.x < a.lowerBound.x)
		{
			a.lowerBound.x = b.lowerBound.x;
			changed = true;
		}

		if (b.lowerBound.y < a.lowerBound.y)
		{
			a.lowerBound.y = b.lowerBound.y;
			changed = true;
		}

		if (a.upperBound.x < b.upperBound.x)
		{
			a.upperBound.x = b.upperBound.x;
			changed = true;
		}

		if (a.upperBound.y < b.upperBound.y)
		{
			a.upperBound.y = b.upperBound.y;
			changed = true;
		}

		return changed;
	}

	void Combine(const AABB& other)
	{
		lowerBound = Math::Min(lowerBound, other.lowerBound);
		upperBound = Math::Max(upperBound, other.upperBound);
	}

	Vector2 Center() const
	{
		return { (lowerBound.x + upperBound.x) * 0.5f, (lowerBound.y + upperBound.y) * 0.5f };
	}

	F32 Perimeter() const
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

struct ShapeDef
{
	/// Use this to store application specific shape data.
	void* userData;

	/// The Coulomb (dry) friction coefficient, usually in the range [0,1].
	F32 friction;

	/// The restitution (bounce) usually in the range [0,1].
	F32 restitution;

	/// The density, usually in kg/m^2.
	F32 density;

	/// Collision filtering data.
	Filter filter;

	/// Custom debug draw color.
	U32 customColor;

	/// A sensor shape generates overlap events but never generates a collision response.
	///	Sensors do not collide with other sensors and do not have continuous collision.
	///	Instead use a ray or shape cast for those scenarios.
	bool isSensor;

	/// Enable sensor events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
	bool enableSensorEvents;

	/// Enable contact events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
	bool enableContactEvents;

	/// Enable hit events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
	bool enableHitEvents;

	/// Enable pre-solve contact events for this shape. Only applies to dynamic bodies. These are expensive
	///	and must be carefully handled due to threading. Ignored for sensors.
	bool enablePreSolveEvents;

	/// Normally shapes on static bodies don't invoke contact creation when they are added to the world. This overrides
	///	that behavior and causes contact creation. This significantly slows down static body creation which can be important
	///	when there are many static shapes.
	/// This is implicitly always true for sensors.
	bool forceContactCreation;

	/// Used internally to detect a valid definition. DO NOT SET.
	I32 internalValue;
};

struct Shape
{
	Shape(const ShapeDef& def);

	Shape(const Shape&);
	Shape(Shape&&) noexcept;

	Shape& operator=(const Shape&);
	Shape& operator=(Shape&&) noexcept;

	Vector2 GetShapeCentroid()
	{
		switch (type)
		{
		case SHAPE_TYPE_CAPSULE: return Math::Lerp(capsule.center1, capsule.center2, 0.5f);
		case SHAPE_TYPE_CIRCLE: return circle.center;
		case SHAPE_TYPE_POLYGON: return polygon.centroid;
		case SHAPE_TYPE_SEGMENT: return Math::Lerp(segment.point1, segment.point2, 0.5f);
		case SHAPE_TYPE_CHAIN_SEGMENT: return Math::Lerp(chainSegment.segment.point1, chainSegment.segment.point2, 0.5f);
		default: return Vector2Zero;
		}
	}

	I32 id;
	I32 bodyId;
	I32 prevShapeId;
	I32 nextShapeId;
	ShapeType type;
	F32 density;
	F32 friction;
	F32 restitution;

	AABB aabb;
	AABB fatAABB;
	Vector2 localCentroid;
	I32 proxyKey;

	Filter filter;
	void* userData;
	U32 customColor;

	union
	{
		Capsule capsule;
		Circle circle;
		Polygon polygon;
		Segment segment;
		ChainSegment chainSegment;
	};

	bool isSensor;
	bool enableSensorEvents;
	bool enableContactEvents;
	bool enableHitEvents;
	bool enablePreSolveEvents;
	bool enlargedAABB;
	bool isFast;
};

struct SegmentDistanceResult
{
	Vector2 closest1;
	Vector2 closest2;
	F32 fraction1;
	F32 fraction2;
	F32 distanceSquared;
};

struct ChainShape
{
	I32 id;
	I32 bodyId;
	I32 nextChainId;
	I32* shapeIndices;
	I32 count;
	U16 revision;
};

struct ShapeExtent
{
	F32 minExtent;
	F32 maxExtent;
};

struct DistanceProxy
{
	Vector2 points[MaxPolygonVertices];
	I32 count;
	F32 radius;
};

struct DistanceCache
{
	U16 count;
	U8 indexA[3];
	U8 indexB[3];
};

struct Transform2D
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

static constexpr Transform2D Transform2DIdentity = { { 0.0f, 0.0f }, { 1.0f, 0.0f } };

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
	I32 iterations;
	I32 simplexCount;
};

struct SimplexVertex
{
	Vector2 wA;
	Vector2 wB;
	Vector2 w;
	F32 a;
	I32 indexA;
	I32 indexB;
};

struct Simplex
{
	SimplexVertex v1, v2, v3;
	I32 count;
};

struct ShapeCastPairInput
{
	DistanceProxy proxyA;
	DistanceProxy proxyB;
	Transform2D transformA;
	Transform2D transformB;
	Vector2 translationB;
	F32 maxFraction;
};

struct BodySim
{
	Transform2D transform;
	Vector2 center;
	Quaternion2 rotation0;
	Vector2 center0;
	Vector2 localCenter;

	Vector2 force;
	F32 torque;

	F32 mass, invMass;

	F32 inertia, invInertia;

	F32 minExtent;
	F32 maxExtent;
	F32 linearDamping;
	F32 angularDamping;
	F32 gravityScale;

	I32 bodyId;

	bool isFast;
	bool isBullet;
	bool isSpeedCapped;
	bool allowFastRotation;
	bool enlargeAABB;
};

struct Sweep
{
	void Create(const BodySim& bodySim);

	Vector2 localCenter;
	Vector2 c1;
	Vector2 c2;
	Quaternion2 q1;
	Quaternion2 q2;
};

struct ContinuousContext
{
	BodySim* fastBodySim;
	Shape* fastShape;
	Vector2 centroid1, centroid2;
	Sweep sweep;
	F32 fraction;
};

struct TOIInput
{
	DistanceProxy proxyA;
	DistanceProxy proxyB;
	Sweep sweepA;
	Sweep sweepB;
	F32 tMax;
};

struct TOIOutput
{
	TOIState state;
	F32 t;
};

struct RigidBody2DDef
{
	/// The body type: static, kinematic, or dynamic.
	BodyType type;

	/// The initial world position of the body. Bodies should be created with the desired position.
	/// @note Creating bodies at the origin and then moving them nearly doubles the cost of body creation, especially
	///	if the body is moved after shapes have been added.
	Vector2 position;

	/// The initial world rotation of the body. Use b2MakeRot() if you have an angle.
	Quaternion2 rotation;

	/// The initial linear velocity of the body's origin. Typically in meters per second.
	Vector2 linearVelocity;

	/// The initial angular velocity of the body. Radians per second.
	F32 angularVelocity;

	/// Linear damping is use to reduce the linear velocity. The damping parameter
	/// can be larger than 1 but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	///	Generally linear damping is undesirable because it makes objects move slowly
	///	as if they are floating.
	F32 linearDamping;

	/// Angular damping is use to reduce the angular velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	///	Angular damping can be use slow down rotating bodies.
	F32 angularDamping;

	/// Scale the gravity applied to this body. Non-dimensional.
	F32 gravityScale;

	/// Sleep velocity threshold, default is 0.05 meter per second
	F32 sleepThreshold;

	/// Use this to store application specific body data.
	void* userData;

	/// Set this flag to false if this body should never fall asleep.
	bool enableSleep;

	/// Is this body initially awake or sleeping?
	bool isAwake;

	/// Should this body be prevented from rotating? Useful for characters.
	bool fixedRotation;

	/// Treat this body as high speed object that performs continuous collision detection
	/// against dynamic and kinematic bodies, but not other bullet bodies.
	///	@warning Bullets should be used sparingly. They are not a solution for general dynamic-versus-dynamic
	///	continuous collision. They may interfere with joint constraints.
	bool isBullet;

	/// Used to disable a body. A disabled body does not move or collide.
	bool isEnabled;

	/// Automatically compute mass and related properties on this body from shapes.
	/// Triggers whenever a shape is add/removed/changed. Default is true.
	bool automaticMass;

	/// This allows this body to bypass rotational speed limits. Should only be used
	///	for circular objects, like wheels.
	bool allowFastRotation;

	/// Used internally to detect a valid definition. DO NOT SET.
	I32 internalValue;
};

struct NH_API RigidBody2D : public Component
{
	RigidBody2D(const RigidBody2DDef& def);

	void Update(Scene* scene);
	void Load(Scene* scene);
	void Cleanup(Scene* scene);

	void* userData;
	I32 setIndex;
	I32 localIndex;
	I32 headContactKey;
	I32 contactCount;
	I32 headShapeId;
	I32 shapeCount;
	I32 headChainId;
	I32 headJointKey;
	I32 jointCount;
	I32 islandId;
	I32 islandPrev;
	I32 islandNext;
	F32 sleepThreshold;
	F32 sleepTime;
	I32 bodyMoveIndex;
	I32 id;

	BodyType type{ BODY_TYPE_DYNAMIC };

	U16 revision;

	bool enableSleep;
	bool fixedRotation;
	bool isSpeedCapped;
	bool isMarked;
	bool automaticMass;
};

struct BodyState
{
	Vector2 linearVelocity;
	F32 angularVelocity;
	I32 flags;
	Vector2 deltaPosition;
	Quaternion2 deltaRotation;
};

constexpr BodyState BodyStateIdentity = { { 0.0f, 0.0f }, 0.0f, 0, { 0.0f, 0.0f }, { 1.0f, 0.0f } };

struct ContactEdge
{
	I32 bodyId;
	I32 prevKey;
	I32 nextKey;
};

struct Contact
{
	I32 setIndex;

	I32 colorIndex;

	I32 localIndex;

	ContactEdge edges[2];
	I32 shapeIdA;
	I32 shapeIdB;

	I32 islandPrev;
	I32 islandNext;
	I32 islandId;

	I32 contactId;

	U32 flags;

	bool isMarked;
};

struct ManifoldPoint
{
	Vector2 point;
	Vector2 anchorA;
	Vector2 anchorB;
	F32 separation;
	F32 normalImpulse;
	F32 tangentImpulse;
	F32 maxNormalImpulse;
	F32 normalVelocity;
	U16 id;
	bool persisted;
};

struct Manifold
{
	ManifoldPoint points[MaxManifoldPoints];
	Vector2 normal;
	U32 pointCount;
};

struct ContactSim
{
	I32 contactId;

	I32 bodySimIndexA;
	I32 bodySimIndexB;

	I32 shapeIdA;
	I32 shapeIdB;

	F32 invMassA;
	F32 invIA;

	F32 invMassB;
	F32 invIB;

	Manifold manifold;

	F32 friction;
	F32 restitution;

	F32 tangentSpeed;

	U32 simFlags;

	DistanceCache cache;
};

struct Softness
{
	void Create(F32 hertz, F32 zeta, F32 h);

	F32 biasRate;
	F32 massScale;
	F32 impulseScale;
};

struct JointEdge
{
	I32 bodyId;
	I32 prevKey;
	I32 nextKey;
};

struct Joint
{
	void* userData;

	// index of simulation set stored in b2World
	// NullIndex when slot is free
	I32 setIndex;

	// index into the constraint graph color array, may be NullIndex for sleeping/disabled joints
	// NullIndex when slot is free
	I32 colorIndex;

	// joint index within set or graph color
	// NullIndex when slot is free
	I32 localIndex;

	JointEdge edges[2];

	I32 jointId;
	I32 islandId;
	I32 islandPrev;
	I32 islandNext;

	// This is monotonically advanced when a body is allocated in this slot
	// Used to check for invalid b2JointId
	I32 revision;

	F32 drawSize;

	JointType type;
	bool isMarked;
	bool collideConnected;

};

struct DistanceJoint
{
	F32 length;
	F32 hertz;
	F32 dampingRatio;
	F32 minLength;
	F32 maxLength;

	F32 maxMotorForce;
	F32 motorSpeed;

	F32 impulse;
	F32 lowerImpulse;
	F32 upperImpulse;
	F32 motorImpulse;

	I32 indexA;
	I32 indexB;
	Vector2 anchorA;
	Vector2 anchorB;
	Vector2 deltaCenter;
	Softness distanceSoftness;
	F32 axialMass;

	bool enableSpring;
	bool enableLimit;
	bool enableMotor;
};

struct MotorJoint
{
	Vector2 linearOffset;
	F32 angularOffset;
	Vector2 linearImpulse;
	F32 angularImpulse;
	F32 maxForce;
	F32 maxTorque;
	F32 correctionFactor;

	I32 indexA;
	I32 indexB;
	Vector2 anchorA;
	Vector2 anchorB;
	Vector2 deltaCenter;
	F32 deltaAngle;
	Matrix2 linearMass;
	F32 angularMass;
};

struct MouseJoint
{
	Vector2 targetA;
	F32 hertz;
	F32 dampingRatio;
	F32 maxForce;

	Vector2 linearImpulse;
	F32 angularImpulse;

	Softness linearSoftness;
	Softness angularSoftness;
	I32 indexB;
	Vector2 anchorB;
	Vector2 deltaCenter;
	Matrix2 linearMass;
};

struct PrismaticJoint
{
	Vector2 localAxisA;
	Vector2 impulse;
	F32 springImpulse;
	F32 motorImpulse;
	F32 lowerImpulse;
	F32 upperImpulse;
	F32 hertz;
	F32 dampingRatio;
	F32 maxMotorForce;
	F32 motorSpeed;
	F32 referenceAngle;
	F32 lowerTranslation;
	F32 upperTranslation;

	I32 indexA;
	I32 indexB;
	Vector2 anchorA;
	Vector2 anchorB;
	Vector2 axisA;
	Vector2 deltaCenter;
	F32 deltaAngle;
	F32 axialMass;
	Softness springSoftness;

	bool enableSpring;
	bool enableLimit;
	bool enableMotor;
};

struct RevoluteJoint
{
	Vector2 linearImpulse;
	F32 springImpulse;
	F32 motorImpulse;
	F32 lowerImpulse;
	F32 upperImpulse;
	F32 hertz;
	F32 dampingRatio;
	F32 maxMotorTorque;
	F32 motorSpeed;
	F32 referenceAngle;
	F32 lowerAngle;
	F32 upperAngle;

	I32 indexA;
	I32 indexB;
	Vector2 anchorA;
	Vector2 anchorB;
	Vector2 deltaCenter;
	F32 deltaAngle;
	F32 axialMass;
	Softness springSoftness;

	bool enableSpring;
	bool enableMotor;
	bool enableLimit;
};

struct WeldJoint
{
	F32 referenceAngle;
	F32 linearHertz;
	F32 linearDampingRatio;
	F32 angularHertz;
	F32 angularDampingRatio;

	Softness linearSoftness;
	Softness angularSoftness;
	Vector2 linearImpulse;
	F32 angularImpulse;

	I32 indexA;
	I32 indexB;
	Vector2 anchorA;
	Vector2 anchorB;
	Vector2 deltaCenter;
	F32 deltaAngle;
	F32 axialMass;
};

struct WheelJoint
{
	Vector2 localAxisA;
	F32 perpImpulse;
	F32 motorImpulse;
	F32 springImpulse;
	F32 lowerImpulse;
	F32 upperImpulse;
	F32 maxMotorTorque;
	F32 motorSpeed;
	F32 lowerTranslation;
	F32 upperTranslation;
	F32 hertz;
	F32 dampingRatio;

	I32 indexA;
	I32 indexB;
	Vector2 anchorA;
	Vector2 anchorB;
	Vector2 axisA;
	Vector2 deltaCenter;
	F32 perpMass;
	F32 motorMass;
	F32 axialMass;
	Softness springSoftness;

	bool enableSpring;
	bool enableMotor;
	bool enableLimit;
};

struct JointSim
{
	JointSim();

	JointSim(const JointSim& other);
	JointSim(JointSim&& other) noexcept;

	JointSim& operator=(const JointSim& other);
	JointSim& operator=(JointSim&& other) noexcept;

	I32 jointId;

	I32 bodyIdA;
	I32 bodyIdB;

	JointType type;

	// Anchors relative to body origin
	Vector2 localOriginAnchorA;
	Vector2 localOriginAnchorB;

	F32 invMassA, invMassB;
	F32 invIA, invIB;

	union
	{
		DistanceJoint distanceJoint;
		MotorJoint motorJoint;
		MouseJoint mouseJoint;
		RevoluteJoint revoluteJoint;
		PrismaticJoint prismaticJoint;
		WeldJoint weldJoint;
		WheelJoint wheelJoint;
	};
};

struct ContactConstraintPoint
{
	Vector2 anchorA, anchorB;
	F32 baseSeparation;
	F32 relativeVelocity;
	F32 normalImpulse;
	F32 tangentImpulse;
	F32 maxNormalImpulse;
	F32 normalMass;
	F32 tangentMass;
};

struct ContactConstraint
{
	I32 indexA;
	I32 indexB;
	ContactConstraintPoint points[2];
	Vector2 normal;
	F32 invMassA, invMassB;
	F32 invIA, invIB;
	F32 friction;
	F32 restitution;
	Softness softness;
	I32 pointCount;
};

struct ContactConstraintSIMD
{
	I32 indexA[NH_SIMD_WIDTH];
	I32 indexB[NH_SIMD_WIDTH];

	FloatW invMassA, invMassB;
	FloatW invIA, invIB;
	Vector2W normal;
	FloatW friction;
	FloatW biasRate;
	FloatW massScale;
	FloatW impulseScale;
	Vector2W anchorA1, anchorB1;
	FloatW normalMass1, tangentMass1;
	FloatW baseSeparation1;
	FloatW normalImpulse1;
	FloatW maxNormalImpulse1;
	FloatW tangentImpulse1;
	Vector2W anchorA2, anchorB2;
	FloatW baseSeparation2;
	FloatW normalImpulse2;
	FloatW maxNormalImpulse2;
	FloatW tangentImpulse2;
	FloatW normalMass2, tangentMass2;
	FloatW restitution;
	FloatW relativeVelocity1, relativeVelocity2;
};

struct SimdBody
{
	Vector2W v;
	FloatW w;
	FloatW flags;
	Vector2W dp;
	Quaternion2W dq;
};

struct GraphColor
{
	// This bitset is indexed by bodyId so this is over-sized to encompass static bodies
	// however I never traverse these bits or use the bit count for anything
	// This bitset is unused on the overflow color.
	Bitset bodySet;

	// cache friendly arrays
	Vector<ContactSim> contactSims;
	Vector<JointSim> jointSims;

	// transient
	union
	{
		ContactConstraintSIMD* simdConstraints;
		ContactConstraint* overflowConstraints;
	};
};

struct ConstraintGraph
{
	void Create(I32 bodyCapacity);
	void Destroy();

	void AddContact(ContactSim& contactSim, Contact& contact);
	void RemoveContact(I32 bodyIdA, I32 bodyIdB, I32 colorIndex, I32 localIndex);
	I32 AssignJointColor(I32 bodyIdA, I32 bodyIdB, bool staticA, bool staticB);
	void AddJoint(JointSim& jointSim, Joint& joint);
	void RemoveJoint(I32 bodyIdA, I32 bodyIdB, I32 colorIndex, I32 localIndex);

	// including overflow at the end
	GraphColor colors[GraphColorCount];
};

struct SolverBlock
{
	I32 startIndex;
	I16 count;
	I16 blockType; // b2SolverBlockType
	// todo consider false sharing of this atomic
	I32_Atomic syncIndex;
};

// Each stage must be completed before going to the next stage.
// Non-iterative stages use a stage instance once while iterative stages re-use the same instance each iteration.
struct SolverStage
{
	SolverStageType type;
	SolverBlock* blocks;
	I32 blockCount;
	I32 colorIndex;
	// todo consider false sharing of this atomic
	I32_Atomic completionCount;
};

// Context for a time step. Recreated each time step.
struct StepContext
{
	// time step
	F32 dt;

	// inverse time step (0 if dt == 0).
	F32 inv_dt;

	// sub-step
	F32 h;
	F32 inv_h;

	I32 subStepCount;

	Softness jointSoftness;
	Softness contactSoftness;
	Softness staticSoftness;

	F32 restitutionThreshold;
	F32 maxLinearVelocity;

	ConstraintGraph* graph;

	// shortcut to body states from awake set
	BodyState* states;

	// shortcut to body sims from awake set
	BodySim* sims;

	// array of all shape ids for shapes that have enlarged AABBs
	I32* enlargedShapes;
	I32 enlargedShapeCount;

	// Array of fast bodies that need continuous collision handling
	I32* fastBodies;
	I32_Atomic fastBodyCount; //_Atomic?

	// Array of bullet bodies that need continuous collision handling
	I32* bulletBodies;
	I32_Atomic bulletBodyCount; //_Atomic?

	// joint pointers for simplified parallel-for access.
	JointSim** joints;

	// contact pointers for simplified parallel-for access.
	// - parallel-for collide with no gaps
	// - parallel-for prepare and store contacts with NULL gaps for SIMD remainders
	// despite being an array of pointers, these are contiguous sub-arrays corresponding
	// to constraint graph colors
	Vector<ContactSim*> contacts;

	ContactConstraintSIMD* simdContactConstraints;
	I32 activeColorCount;
	I32 workerCount;

	SolverStage* stages;
	I32 stageCount;
	bool enableWarmStarting;

	// todo padding to prevent false sharing
	U8 dummy1[64];

	// sync index (16-bits) | stage type (16-bits)
	U32_Atomic atomicSyncBits; //_Atomic?

	U8 dummy2[64];
};

struct WorkerContext
{
	StepContext* context;
	int workerIndex;
	void* userTask;
};

struct Island
{
	// index of solver set stored in b2World
	// may be NullIndex
	I32 setIndex;

	// island index within set
	// may be NullIndex
	I32 localIndex;

	I32 islandId;

	I32 headBody;
	I32 tailBody;
	I32 bodyCount;

	I32 headContact;
	I32 tailContact;
	I32 contactCount;

	I32 headJoint;
	I32 tailJoint;
	I32 jointCount;

	// Union find
	I32 parentIsland;

	// Keeps track of how many contacts have been removed from this island.
	// This is used to determine if an island is a candidate for splitting.
	I32 constraintRemoveCount;
};

struct IslandSim
{
	I32 islandId;
};

struct SolverSet
{
	void Destroy();

	// Body array. Empty for unused set.
	Vector<BodySim> bodySims;

	// Body state only exists for active set
	Vector<BodyState> bodyStates;

	// This holds sleeping/disabled joints. Empty for static/active set.
	Vector<JointSim> jointSims;

	// This holds all contacts for sleeping sets.
	// This holds non-touching contacts for the awake set.
	Vector<ContactSim> contactSims;

	// The awake set has an array of islands. Sleeping sets normally have a single islands. However, joints
	// created between sleeping sets causes the sets to merge, leaving them with multiple islands. These sleeping
	// islands will be naturally merged with the set is woken.
	// The static and disabled sets have no islands.
	// Islands live in the solver sets to limit the number of islands that need to be considered for sleeping.
	Vector<IslandSim> islandSims;

	// Aligns with b2World::solverSetIdPool. Used to create a stable id for body/contact/joint/islands.
	I32 setIndex;
};

struct TaskContext
{
	// These bits align with the b2ConstraintGraph::contactBlocks and signal a change in contact status
	Bitset contactStateBitset;

	// Used to track bodies with shapes that have enlarged AABBs. This avoids having a bit array
	// that is very large when there are many static shapes.
	Bitset enlargedSimBitset;

	// Used to put islands to sleep
	Bitset awakeIslandBitset;

	// Per worker split island candidate
	F32 splitSleepTime;
	I32 splitIslandId;
};

struct MovePair
{
	I32 shapeIndexA;
	I32 shapeIndexB;
	MovePair* next;
	bool heap;
};

struct MoveResult
{
	MovePair* pairList;
};

struct QueryPairContext
{
	MoveResult* moveResult;
	BodyType queryTreeType;
	I32 queryProxyKey;
	I32 queryShapeIndex;
};

struct SeparationFunction
{
	const DistanceProxy* proxyA;
	const DistanceProxy* proxyB;
	Sweep sweepA, sweepB;
	Vector2 localPoint;
	Vector2 axis;
	SeparationType type;
};