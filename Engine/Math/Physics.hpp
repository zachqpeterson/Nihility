#pragma once

#include "Math.hpp"
#include "Resources\Component.hpp"
#include "Resources\Scene.hpp"

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

struct Scene;

struct RigidBody2D : public Component
{
	RigidBody2D() = default;
	RigidBody2D(RigidBody2D&& other) noexcept : Component(Move(other)), position{ other.position }, velocity{ other.velocity }, acceleration{ other.acceleration },
		rotation{ other.rotation }, angularMomentum{ other.angularMomentum }, layers{ other.layers }, invMass{ other.invMass }, restitution{ other.restitution },
		simulated{ other.simulated }, trigger{ other.trigger }, collider{ other.collider }, event{ other.event }
	{
	}

	RigidBody2D& operator=(RigidBody2D&& other) noexcept
	{
		Component::operator=(Move(other));
		position = other.position;
		velocity = other.velocity;
		acceleration = other.acceleration;
		rotation = other.rotation;
		angularMomentum = other.angularMomentum;
		layers = other.layers;
		invMass = other.invMass;
		restitution = other.restitution;
		simulated = other.simulated;
		trigger = other.trigger;
		collider = other.collider;
		event = other.event;

		return *this;
	}

	virtual void Update(Scene* scene) final
	{
		Entity* e = scene->GetEntity(entityID);
		e->transform.SetPosition((Vector3)position);
		//e->transform.SetRotation((Quaternion3)rotation);
	}
	virtual void Load(Scene* scene) final {}
	virtual void Cleanup(Scene* scene) final {}

	void SetPosition(const Vector2& newPosition) { position = newPosition; }
	void SetVelocity(const Vector2& newVelocity) { velocity = newVelocity; }
	void AddForce(const Vector2& force) { acceleration += force * invMass; }
	void SetSimulated(bool b) { simulated = b; invMass = 0.0f; }
	void SetTrigger(bool b) { trigger = b; }
	void SetMass(F32 mass) { if (mass <= 0.0f || !simulated) { invMass = 0.0f; } else { invMass = 1.0f / mass; } }
	void SetRestitution(F32 newRestitution) { restitution = newRestitution; }
	void SetCollider(const Collider2D& newCollider) { collider = newCollider; }

private:
	Vector2 position{ Vector2Zero };
	Vector2 velocity{ Vector2Zero };
	Vector2 acceleration{ Vector2Zero };

	Quaternion2 rotation{ Quaternion2Identity };
	F32 angularMomentum{ 0.0f };

	U64 layers{ 1 };
	F32 invMass{ 1.0f };
	F32 restitution{ 0.0f };
	bool simulated{ true };
	bool trigger{ false };

	Collider2D collider{};

	PhysicsEvent2D event{ nullptr };

	friend class Physics;
};

struct Manifold2D
{
	RigidBody2D* rb0;
	RigidBody2D* rb1;
	F32 penetration;
	Vector2 normal;
};

struct Scene;

class NH_API Physics
{
private:
	static bool Initialize();
	static void Shutdown();

	static void SetScene(Scene* scene);
	static void Update(F64 step);

	static bool DetectCollision(Manifold2D& manifold);
	static void ResolveCollision(Manifold2D& manifold);
	static void CorrectPosition(Manifold2D& manifold);

	static Scene* scene;
	static Vector<RigidBody2D>* bodies;
	static Vector<Manifold2D> manifolds;
	static Vector3 gravity;

	STATIC_CLASS(Physics);
	friend class Engine;
	friend struct Scene;
};