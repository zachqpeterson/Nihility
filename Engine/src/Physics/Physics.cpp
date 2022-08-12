#include "Physics.hpp"

#include "BAH.hpp"
#include "Containers/Vector.hpp"

#include "Core/Time.hpp"

HashMap<U64, PhysicsObject2D*> Physics::physicsObjects2D;
HashMap<U64, PhysicsObject3D*> Physics::physicsObjects3D;

BAH Physics::tree;

F64 Physics::airDensity = 1.29;
F64 Physics::gravity = 9.807;

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

void Physics::Update(F64 step)
{
	List<PhysicsObject2D*> objects;

	for (List<HashMap<U64, PhysicsObject2D*>::Node>& l : physicsObjects2D)
	{
		for (HashMap<U64, PhysicsObject2D*>::Node& n : l)
		{
			if (!n.value->kinematic)
			{
				PhysicsObject2D& po = *n.value;
				po.velocity += Vector2::UP * (F32)(gravity * po.gravityScale * po.mass * step);
				po.velocity += po.force * (F32)po.massInv;
				po.velocity += -po.velocity.Normalized() * po.velocity.SqrMagnitude() * (F32)(po.dragCoefficient * po.area * 0.5 * airDensity * step);

				po.prevPosition = po.transform->Position();
				po.transform->Translate(po.velocity);

				po.force = Vector2::ZERO;
			}

			objects.PushBack(n.value);
		}
	}

	List<PhysicsObject2D*> copy = objects;

	tree.Build(Move(copy));

	List<Manifold2D> contacts;
	BroadPhase(tree, objects, contacts);
	NarrowPhase(contacts);
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
		po->dragCoefficient = 2.05;

		po->area = (config.xBounds.y - config.xBounds.x) * (config.yBounds.y - config.yBounds.x);
	} break;
	case COLLIDER_TYPE_CIRCLE: {
		CircleCollider* collider = (CircleCollider*)Memory::Allocate(sizeof(CircleCollider), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		collider->radius = config.radius;
		collider->xBounds = { (F32)-config.radius, (F32)config.radius };
		collider->yBounds = { (F32)-config.radius, (F32)config.radius };
		po->collider = collider;
		po->dragCoefficient = 1.17;

		po->area = (F32)(PI * config.radius * config.radius);
	} break;
	case COLLIDER_TYPE_CAPSULE: {
		CapsuleCollider2D* collider = (CapsuleCollider2D*)Memory::Allocate(sizeof(CapsuleCollider2D), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		po->collider = collider;
		po->dragCoefficient = 1.6;

	} break;
	case COLLIDER_TYPE_POLYGON: {
		PolygonCollider* collider = (PolygonCollider*)Memory::Allocate(sizeof(PolygonCollider), MEMORY_TAG_DATA_STRUCT);
		collider->trigger = config.trigger;
		po->collider = collider;
		po->dragCoefficient = 1.0;

		//TODO: A set is convex if given any two points P and Q in the set, the line segment (1 - t)P + tQ for t elm. [0, 1] is also in the set
	} break;
	case COLLIDER_TYPE_NONE:
	default: {
		po->area = 0.0f;
	} break;
	}

	po->collider->type = config.type;
	po->collider->trigger = config.trigger;
	po->transform = config.transform;
	po->prevPosition = po->transform->Position();
	po->prevRotation = po->transform->Rotation().angle;

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
	U64 size = objects.Size() * objects.Size();
	bool* freeContacts = (bool*)Memory::Allocate(size, MEMORY_TAG_DATA_STRUCT);

	//Timer t;
	//t.Start();
	for (PhysicsObject2D* po0 : objects)
	{
		results.Clear();

		tree.Query(po0, results);

		for (PhysicsObject2D* po1 : results)
		{
			U64 id0 = po0->id;
			U64 id1 = po1->id;
			bool& free0 = freeContacts[id0 + id1 * objects.Size()];
			bool& free1 = freeContacts[id1 + id0 * objects.Size()];

			if (po0->layerMask & po1->layerMask && id0 != id1 && !free0 && !free1 && (po0->mass > 0.0f || po1->mass > 0.0f))
			{
				free0 = true;
				free1 = true;
				Manifold2D manifold{ po0, po1 };
				contacts.PushBack(manifold);
			}
		}
	}
	//Logger::Debug(t.CurrentTime());

	Memory::Free(freeContacts, size, MEMORY_TAG_DATA_STRUCT);
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
			case COLLIDER_TYPE_CIRCLE: if (AABBvsCircle(m)) { ResolveCollision(m); }
									 break;
			case COLLIDER_TYPE_CAPSULE:
				break;
			case COLLIDER_TYPE_POLYGON:
				break;
			} break;
		case COLLIDER_TYPE_CIRCLE:
			switch (m.b->collider->type)
			{
			case COLLIDER_TYPE_RECTANGLE: if (CirclevsAABB(m)) { ResolveCollision(m); }
										break;
			case COLLIDER_TYPE_CIRCLE: if (CirclevsCircle(m)) { ResolveCollision(m); }
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

	F64 radius = aCollider->radius + bCollider->radius;
	F64 radiusSqr = radius * radius;

	if (distance > radiusSqr) { return false; }

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

	Vector<Vector2> v0;
	v0.Push(a->transform->Position() + Vector2{ aCollider->xBounds.x, aCollider->yBounds.x });
	v0.Push(a->transform->Position() + Vector2{ aCollider->xBounds.x, aCollider->yBounds.y });
	v0.Push(a->transform->Position() + Vector2{ aCollider->xBounds.y, aCollider->yBounds.y });
	v0.Push(a->transform->Position() + Vector2{ aCollider->xBounds.y, aCollider->yBounds.x });

	Vector<Vector2> v1;
	v1.Push(b->transform->Position() + Vector2{ bCollider->xBounds.x, bCollider->yBounds.x });
	v1.Push(b->transform->Position() + Vector2{ bCollider->xBounds.x, bCollider->yBounds.y });
	v1.Push(b->transform->Position() + Vector2{ bCollider->xBounds.y, bCollider->yBounds.y });
	v1.Push(b->transform->Position() + Vector2{ bCollider->xBounds.y, bCollider->yBounds.x });

	if (TestIntersection(v0, v1))
	{
		return true;
	}

	return false;

	//PhysicsObject2D* a = m.a;
	//PhysicsObject2D* b = m.b;
	//RectangleCollider* aCollider = (RectangleCollider*)a->collider;
	//RectangleCollider* bCollider = (RectangleCollider*)b->collider;
	//
	//Vector2 direction = a->velocity + b->velocity;
	//Vector2 aSize = Vector2(aCollider->xBounds.y - aCollider->xBounds.x, aCollider->yBounds.y - aCollider->yBounds.x);
	//Vector2 bExpandedNear = b->prevPosition + Vector2(bCollider->xBounds.x, bCollider->yBounds.x) - aSize / 2.0f;
	//Vector2 bExpandedFar = Vector2(bCollider->xBounds.y - bCollider->xBounds.x, bCollider->yBounds.y - bCollider->yBounds.x) + aSize;
	//
	//Vector2 tNear = (bExpandedNear - a->prevPosition) / direction;
	//Vector2 tFar = (bExpandedNear + bExpandedFar - a->prevPosition) / direction;
	//
	//if (tNear.x > tFar.x) { Math::Swap(tNear.x, tFar.x); }
	//if (tNear.y > tFar.y) { Math::Swap(tNear.y, tFar.y); }
	//
	//F32 tHitNear = Math::Max(tNear.x, tNear.y);
	//F32 tHitFar = Math::Max(tFar.x, tFar.y);
	//
	////TODO: Collide if it's inside
	//if ((tNear.x > tFar.y || tNear.y > tFar.x || tHitNear > 1.0f || tFar.x < 0 || tFar.y < 0)) { return false; }
	//
	//bool xColl = tNear.x > tNear.y;
	//bool left = direction.x < 0;
	//bool top = direction.y < 0;
	//m.normal = ((Vector2::LEFT * left + Vector2::RIGHT * !left) * xColl) + ((Vector2::DOWN * top + Vector2::UP * !top) * !xColl);
	//
	//m.penetration = m.normal.Dot(direction) * (1.0f - tHitNear);
	//
	//return true;
}

bool Physics::AABBvsCircle(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;
	RectangleCollider* aCollider = (RectangleCollider*)a->collider;
	CircleCollider* bCollider = (CircleCollider*)b->collider;
	Vector2 aPos = a->transform->Position();
	Vector2 bPos = b->transform->Position();

	Vector2 closestPoint = bPos + (aPos - bPos).Clamped(aCollider->xBounds, aCollider->yBounds);
	Vector2 checkX = aCollider->xBounds + aPos.x;
	Vector2 checkY = aCollider->yBounds + aPos.y;

	if ((closestPoint.x < checkX.y && closestPoint.x > checkX.x) &&
		(closestPoint.y < checkY.y && closestPoint.y > checkY.x))
	{
		Vector2 closestBound = closestPoint.Closest(checkX, checkY);

		if (Math::Abs(closestBound.x - closestPoint.x) < Math::Abs(closestBound.y - closestPoint.y))
		{
			closestPoint.x = closestBound.x;
		}
		else
		{
			closestPoint.y = closestBound.y;
		}
	}

	Vector2 normal = (closestPoint - aPos);
	F32 distance = normal.SqrMagnitude();

	if (distance < bCollider->radius * bCollider->radius)
	{
		distance = Math::Sqrt(distance);

		m.normal = normal.Normalize();
		m.penetration = bCollider->radius - distance;

		return true;
	}

	return false;
}

bool Physics::CirclevsAABB(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;
	CircleCollider* aCollider = (CircleCollider*)a->collider;
	RectangleCollider* bCollider = (RectangleCollider*)b->collider;

	Vector2 n = b->transform->Position() - a->transform->Position();

	F32 xExtent = (bCollider->xBounds.y - bCollider->xBounds.x) * 0.5f;

	F32 xOverlap = (F32)aCollider->radius + xExtent - Math::Abs(n.x);

	if (xOverlap > 0.0f)
	{
		F32 yExtent = (bCollider->yBounds.y - bCollider->yBounds.x) * 0.5f;

		F32 yOverlap = (F32)aCollider->radius + yExtent - Math::Abs(n.y);

		if (yOverlap > 0.0f)
		{
			if (yOverlap > xOverlap)
			{
				m.penetration = xOverlap;
				m.normal = n.x < 0.0f ? Vector2::RIGHT : Vector2::LEFT;
			}
			else
			{
				m.penetration = yOverlap;
				m.normal = n.y < 0.0f ? Vector2::DOWN : Vector2::UP;
			}

			return true;
		}
	}

	return false;
}

void Physics::ResolveCollision(Manifold2D& m)
{
	PhysicsObject2D* a = m.a;
	PhysicsObject2D* b = m.b;

	Vector2 relVelocity = b->velocity - a->velocity;

	F32 velAlongNormal = relVelocity.Dot(m.normal);

	if (velAlongNormal > 0.0f || a->massInv + b->massInv <= FLOAT_EPSILON) { return; }

	F64 restitution = Math::Min(a->restitution, b->restitution);
	F64 j = (-(1.0 + restitution) * velAlongNormal) / (a->massInv + b->massInv);

	Vector2 impulse = m.normal * (F32)j;

	a->velocity -= impulse * (F32)a->massInv;
	b->velocity += impulse * (F32)b->massInv;

	F64 slop = 0.001;
	Vector2 correction = m.normal * (F32)(Math::Max(m.penetration - slop, 0.0) / (a->massInv + b->massInv));
	a->transform->Translate(-correction * (F32)a->massInv);
	b->transform->Translate(correction * (F32)b->massInv);
}

bool Physics::OverlapRect(const Vector2& boundsX, const Vector2& boundsY, List<PhysicsObject2D*>& results)
{
	if (tree.root)
	{
		tree.Query(boundsX, boundsY, results);

		return results.Size();
	}

	return false;
}

I32 Physics::WhichSide(const Vector<Vector2>& vertices, const Vector2& p, const Vector2& d)
{
	U32 pos = 0;
	U32 neg = 0;
	for (Vector2& v : vertices)
	{
		F32 t = d.Dot(v - p);
		pos += t > 0.0f;
		neg += t < 0.0f;
		if (pos && neg) { return 0; }
	}

	return pos ? 1 : -1;
}

bool Physics::TestIntersection(const Vector<Vector2>& vertices0, const Vector<Vector2>& vertices1, const Vector2& velocity, F32& tFirst, F32& tLast)
{
	tFirst = -F32_MAX;
	tLast = F32_MAX;

	for (auto it0 = vertices0.begin(), it1 = vertices1.begin(); it0 != vertices0.end() && it1 != vertices1.end(); ++it0, ++it1)
	{
		const Vector2& v0 = *it0;
		const Vector2& v1 = *it1;

		Vector2 direction = v1 - v0;

		F32 min0 = Math::Min(direction.Dot(v0));
	}
}