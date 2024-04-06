#pragma once

#include "Math.hpp"
#include "Resources\Component.hpp"

enum PhysicsEventType
{
	PHYSICS_EVENT_ON_COLLISION,
	PHYSICS_EVENT_ON_TRIGGER_ENTER,
	PHYSICS_EVENT_ON_TRIGGER_EXIT,
};

struct Manifold2D;
struct Manifold3D;

typedef bool(*PhysicsEvent2D)(const Manifold2D&, PhysicsEventType);
typedef bool(*PhysicsEvent3D)(const Manifold3D&, PhysicsEventType);

enum ColliderType
{
	COLLIDER_TYPE_AABB,
	COLLIDER_TYPE_CIRCLE,
};

struct Collider3D
{

};

struct Collider2D
{
	ColliderType type;

	struct AABB
	{
		F32 halfWidth;
		F32 halfHeight;
	};

	struct Circle
	{
		F32 radius;
	};

	union
	{
		AABB aabb;
		Circle circle;
	};
};

struct RigidBody3D : public Component
{
	Vector3 position;
	Vector3 velocity;

	Quaternion3 rotation;
	Vector3 angularMomentum;

	F32 invMass;
	F32 restitution;
	bool simulated;
	bool trigger;

	Collider3D collider;

	PhysicsEvent3D event;
};

struct RigidBody2D : public Component
{
	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;

	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
	Vector2 jerk;

	Quaternion2 rotation;
	F32 angularMomentum;

	U64 layers{ 1 };
	F32 invMass;
	F32 restitution;
	bool simulated;
	bool trigger;

	Collider2D collider;

	PhysicsEvent2D event;
};

struct Manifold2D
{
	RigidBody2D* rb0;
	RigidBody2D* rb1;
	F32 penetration;
	Vector2 normal;
};

class NH_API Physics
{
private:
	static bool Initialize();
	static void Shutdown();

	static void Update(F64 step);

	static bool DetectCollision(Manifold2D& manifold);
	static void ResolveCollision(Manifold2D& manifold);
	static void CorrectPosition(Manifold2D& manifold);

	static Vector<RigidBody2D> bodies;
	static Vector<Manifold2D> manifolds;
	static Vector3 gravity;

	STATIC_CLASS(Physics);
	friend class Engine;
};