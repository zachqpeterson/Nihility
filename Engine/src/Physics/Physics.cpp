#include "Physics.hpp"

#include "BAH.hpp"
#include "Containers/Vector.hpp"
#include "Core/Time.hpp"

HashMap<U64, PhysicsObject2D*> Physics::physicsObjects2D;
HashMap<U64, PhysicsObject3D*> Physics::physicsObjects3D;

F32 Physics::airPressure = 1.0f;
F32 Physics::gravity = 9.8f;

bool Physics::Initialize()
{
	PhysicsObject2D* invalidPO2D = (PhysicsObject2D*)Memory::Allocate(sizeof(PhysicsObject2D), MEMORY_TAG_DATA_STRUCT);
	invalidPO2D->id = U64_MAX;
	physicsObjects2D = Move(HashMap<U64, PhysicsObject2D*>(25, invalidPO2D));

	PhysicsObject3D* invalidPO3D = (PhysicsObject3D*)Memory::Allocate(sizeof(PhysicsObject3D), MEMORY_TAG_DATA_STRUCT);
	invalidPO3D->id = U64_MAX;
	physicsObjects3D = Move(HashMap<U64, PhysicsObject3D*>(25, invalidPO3D));

	return true;
}

void Physics::Shutdown()
{
	for (List<HashMap<U64, PhysicsObject2D*>::Node>& l : physicsObjects2D)
	{
		for (HashMap<U64, PhysicsObject2D*>::Node& n : l)
		{
			Memory::Free(n.value, sizeof(PhysicsObject2D), MEMORY_TAG_DATA_STRUCT);
		}

		l.Clear();
	}

	physicsObjects2D.Destroy();

	for (List<HashMap<U64, PhysicsObject3D*>::Node>& l : physicsObjects3D)
	{
		for (HashMap<U64, PhysicsObject3D*>::Node& n : l)
		{
			Memory::Free(n.value, sizeof(PhysicsObject3D), MEMORY_TAG_DATA_STRUCT);
		}

		l.Clear();
	}

	physicsObjects3D.Destroy();
}

void Physics::Update()
{
	List<PhysicsObject2D*> objects;

	for (List<HashMap<U64, PhysicsObject2D*>::Node>& l : physicsObjects2D)
	{
		for (HashMap<U64, PhysicsObject2D*>::Node& n : l)
		{
			if (!n.value->kinematic) { n.value->force += Vector2::UP * gravity * n.value->gravityScale * n.value->mass; }
			objects.PushBack(n.value);
		}
	}

	List<PhysicsObject2D*> copy = objects;

	BAH tree;
	tree.Build(Move(copy));

	List<Manifold2D> contacts;
	BroadPhase(tree, objects, contacts);
	NarrowPhase(contacts);

	for (PhysicsObject2D* po : objects)
	{
		if (po->kinematic) { continue; }

		po->velocity += po->force * po->massInv * (F32)Time::DeltaTime();
		po->transform->Translate(po->velocity);

		po->velocity += -po->velocity.Normalized() * po->velocity.SqrMagnitude() * po->dragCoefficient * po->area * 0.5f * airPressure * (F32)Time::DeltaTime();

		po->force = Vector2::ZERO;
	}
}

PhysicsObject2D* Physics::Create2DPhysicsObject(PhysicsObject2DConfig& config)
{
	static U64 id = 0;

	PhysicsObject2D* po = (PhysicsObject2D*)Memory::Allocate(sizeof(PhysicsObject2D), MEMORY_TAG_DATA_STRUCT);
	po->id = id;
	++id;

	switch (config.type)
	{
	case COLLIDER_TYPE_RECTANGLE: {
		RectangleCollider* collider = (RectangleCollider*)Memory::Allocate(sizeof(RectangleCollider), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		collider->xBounds = config.xBounds;
		collider->yBounds = config.yBounds;
		po->collider = collider;

		po->area = (config.xBounds.y - config.xBounds.x) * (config.yBounds.y - config.yBounds.x);
	} break;
	case COLLIDER_TYPE_CIRCLE: {
		CircleCollider* collider = (CircleCollider*)Memory::Allocate(sizeof(CircleCollider), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		collider->radius = config.radius;
		collider->xBounds = { -config.radius, config.radius };
		collider->yBounds = { -config.radius, config.radius };
		po->collider = collider;

		po->area = (F32)(PI * config.radius * config.radius);
	} break;
	case COLLIDER_TYPE_CAPSULE: {
		CapsuleCollider* collider = (CapsuleCollider*)Memory::Allocate(sizeof(CapsuleCollider), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		po->collider = collider;
	} break;
	case COLLIDER_TYPE_POLYGON: {
		PolygonCollider* collider = (PolygonCollider*)Memory::Allocate(sizeof(PolygonCollider), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		po->collider = collider;
	} break;
	case COLLIDER_TYPE_NONE:
	default: {
		po->area = 0.0f;
	} break;
	}

	po->collider->type = config.type;
	po->collider->trigger = config.trigger;
	po->transform = config.transform;

	po->dragCoefficient = config.dragCoefficient;
	po->restitution = config.restitution;
	po->gravityScale = config.gravityScale;
	po->kinematic = config.kinematic;

	po->mass = po->area * config.density;
	if (po->mass == 0.0f) { po->massInv = 0.0f; }
	else { po->massInv = 1.0f / po->mass; }

	//TODO: inertia
	po->inertia = 0.0f;
	if (po->inertia == 0.0f) { po->inertiaInv = 0.0f; }
	else { po->inertiaInv = 1.0f / po->inertia; }

	//TODO: layerMask
	po->layerMask = 1;

	physicsObjects2D.Insert(po->id, po);

	return po;
}

PhysicsObject3D* Physics::Create3DPhysicsObject()
{
	static U64 id = 0;
	PhysicsObject3D* po = (PhysicsObject3D*)Memory::Allocate(sizeof(PhysicsObject3D), MEMORY_TAG_DATA_STRUCT);
	po->id = id;
	++id;

	physicsObjects3D.Insert(po->id, po);

	return po;
}

void Physics::BroadPhase(BAH& tree, List<PhysicsObject2D*>& objects, List<Manifold2D>& contacts)
{
	List<PhysicsObject2D*> results;
	Vector<Vector<bool>> freeContacts(objects.Size(), Move(Vector<bool>(objects.Size(), true)));

	for (PhysicsObject2D* po0 : objects)
	{
		results.Clear();
		tree.Query(po0, results);

		for (PhysicsObject2D* po1 : results)
		{
			U64 id0 = po0->id;
			U64 id1 = po1->id;
			bool& free0 = freeContacts[id0][id1];
			bool& free1 = freeContacts[id1][id0];

			if (po0->layerMask & po1->layerMask && id0 != id1 && free0 && free1)
			{
				free0 = false;
				free1 = false;
				Manifold2D manifold{ po0, po1 };
				contacts.PushBack(manifold);
			}
		}
	}
}

void Physics::NarrowPhase(List<Manifold2D>& contacts)
{
	for (Manifold2D& m : contacts)
	{
		switch (m.a->collider->type)
		{
		case COLLIDER_TYPE_RECTANGLE:
			switch (m.b->collider->type)
			{
			case COLLIDER_TYPE_RECTANGLE: if (AABBvsAABB(m)) { ResolveCollision(m); }
				break;
			case COLLIDER_TYPE_CIRCLE: if(AABBvsCircle(m)) { ResolveCollision(m); }
				break;
			case COLLIDER_TYPE_CAPSULE:
				break;
			case COLLIDER_TYPE_POLYGON:
				break;
			} break;
		case COLLIDER_TYPE_CIRCLE:
			switch (m.b->collider->type)
			{
			case COLLIDER_TYPE_RECTANGLE: if(CirclevsAABB(m)) { ResolveCollision(m); }
				break;
			case COLLIDER_TYPE_CIRCLE: if(CirclevsCircle(m)) { ResolveCollision(m); }
				break;
			case COLLIDER_TYPE_CAPSULE:
				break;
			case COLLIDER_TYPE_POLYGON:
				break;
			} break;
		case COLLIDER_TYPE_CAPSULE:
			switch (m.b->collider->type)
			{
			case COLLIDER_TYPE_RECTANGLE:
				break;
			case COLLIDER_TYPE_CIRCLE:
				break;
			case COLLIDER_TYPE_CAPSULE:
				break;
			case COLLIDER_TYPE_POLYGON:
				break;
			} break;
		case COLLIDER_TYPE_POLYGON:
			switch (m.b->collider->type)
			{
			case COLLIDER_TYPE_RECTANGLE:
				break;
			case COLLIDER_TYPE_CIRCLE:
				break;
			case COLLIDER_TYPE_CAPSULE:
				break;
			case COLLIDER_TYPE_POLYGON:
				break;
			} break;
		}
	}
}

bool Physics::CirclevsCircle(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;
	CircleCollider* aCollider = (CircleCollider*)a->collider;
	CircleCollider* bCollider = (CircleCollider*)b->collider;

	Vector2 direction = (b->transform->Position() - a->transform->Position());

	F32 distance = direction.SqrMagnitude();

	F32 radius = aCollider->radius + aCollider->radius;
	radius *= radius;

	if (distance > radius) { return false; }

	distance = Math::Sqrt(distance);

	if (distance != 0.0f)
	{
		m.penetration = radius - distance;
		m.normal = direction / distance;
	}
	else
	{
		m.penetration = aCollider->radius;
		m.normal = Vector2::RIGHT;
	}

	return true;
}

bool Physics::AABBvsAABB(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;
	RectangleCollider* aCollider = (RectangleCollider*)a->collider;
	RectangleCollider* bCollider = (RectangleCollider*)b->collider;

	Vector2 n = b->transform->Position() - a->transform->Position();

	F32 aExtent = (aCollider->xBounds.y - aCollider->xBounds.x) * 0.5f;
	F32 bExtent = (bCollider->xBounds.y - bCollider->xBounds.x) * 0.5f;

	F32 xOverlap = aExtent + bExtent - Math::Abs(n.x);

	if (xOverlap > 0.0f)
	{
		F32 aExtent = (aCollider->yBounds.y - aCollider->yBounds.x) * 0.5f;
		F32 bExtent = (bCollider->yBounds.y - bCollider->yBounds.x) * 0.5f;

		F32 yOverlap = aExtent + bExtent - Math::Abs(n.y);

		if (yOverlap > 0.0f)
		{
			if (xOverlap > yOverlap)
			{
				if (n.x < 0.0f) { m.normal = Vector2::RIGHT; }
				else
				{
					m.normal = n.Normalized();
					m.penetration = yOverlap;
				}

				return true;
			}
			else
			{
				if (n.y < 0.0f) { m.normal = Vector2::DOWN; }
				else
				{
					m.normal = n.Normalized();
					m.penetration = xOverlap;
				}

				return true;
			}
		}
	}

	return false;
}

bool Physics::AABBvsCircle(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;
	RectangleCollider* aCollider = (RectangleCollider*)a->collider;
	CircleCollider* bCollider = (CircleCollider*)b->collider;

	Vector2 direction = b->transform->Position() - a->transform->Position();
	Vector2 closest = direction;

	F32 xExtent = (aCollider->xBounds.y - aCollider->xBounds.x) * 0.5f;
	F32 yExtent = (aCollider->yBounds.y - aCollider->yBounds.x) * 0.5f;

	closest.x = Math::Clamp(-xExtent, xExtent, closest.x);
	closest.y = Math::Clamp(-yExtent, yExtent, closest.y);

	bool inside = false;

	if (direction == closest)
	{
		inside = true;

		if (Math::Abs(direction.x) > Math::Abs(direction.y))
		{
			if (closest.x > 0.0f) { closest.x = xExtent; }
			else { closest.x = -xExtent; }
		}
		else
		{
			if (closest.y > 0.0f) { closest.y = yExtent; }
			else { closest.y = -yExtent; }
		}
	}

	Vector2 normal = direction - closest;
	F32 d = normal.SqrMagnitude();
	F32 r = bCollider->radius;

	if (d > r * r && !inside) { return false; }

	d = Math::Sqrt(d);

	if (inside)
	{
		m.normal = -normal;
		m.penetration = r - d;
	}
	else
	{
		m.normal = normal;
		m.penetration = r - d;
	}

	return true;
}

bool Physics::CirclevsAABB(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;
	CircleCollider* aCollider = (CircleCollider*)a->collider;
	RectangleCollider* bCollider = (RectangleCollider*)b->collider;

	Vector2 direction = b->transform->Position() - a->transform->Position();
	Vector2 closest = direction;

	F32 xExtent = (bCollider->xBounds.y - bCollider->xBounds.x) * 0.5f;
	F32 yExtent = (bCollider->yBounds.y - bCollider->yBounds.x) * 0.5f;

	closest.x = Math::Clamp(-xExtent, xExtent, closest.x);
	closest.y = Math::Clamp(-yExtent, yExtent, closest.y);

	bool inside = false;

	if (direction == closest)
	{
		inside = true;

		if (Math::Abs(direction.x) > Math::Abs(direction.y))
		{
			if (closest.x > 0.0f) { closest.x = xExtent; }
			else { closest.x = -xExtent; }
		}
		else
		{
			if (closest.y > 0.0f) { closest.y = yExtent; }
			else { closest.y = -yExtent; }
		}
	}

	Vector2 normal = direction - closest;
	F32 d = normal.SqrMagnitude();
	F32 r = aCollider->radius;

	if (d > r * r && !inside) { return false; }

	d = Math::Sqrt(d);

	if (inside)
	{
		m.normal = -normal;
		m.penetration = r - d;
	}
	else
	{
		m.normal = normal;
		m.penetration = r - d;
	}

	return true;
}

void Physics::ResolveCollision(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;

	Vector2 relVelocity = b->velocity - a->velocity;

	F32 velAlongNormal = relVelocity.Dot(m.normal);

	if (velAlongNormal > 0 || a->massInv + b->massInv <= 0.0000001f) { return; }

	F32 restitution = Math::Min(a->restitution, b->restitution);
	F32 j = (-(1.0f + restitution) * velAlongNormal) / (a->massInv + b->massInv);

	Vector2 impulse = m.normal * j;

	a->velocity -= impulse * a->massInv;
	b->velocity += impulse * b->massInv;

	Vector2 correction = m.normal * (Math::Max(m.penetration, 0.0f) / (a->massInv + b->massInv));
	a->transform->Translate(-correction * a->massInv);
	b->transform->Translate(correction * b->massInv);
}