#include "Physics.hpp"

#include "ContactManager.hpp"

#include <Containers/Vector.hpp>

#include "Core/Time.hpp"

List<PhysicsObject2D*> Physics::physicsObjects2D;
List<PhysicsObject3D*> Physics::physicsObjects3D;
Array<Array<Collision2DFn, COLLIDER_2D_MAX>, COLLIDER_2D_MAX> Physics::collision2DTable;

ContactManager* Physics::contactManager;

F64 Physics::airDensity = 1.29;
F64 Physics::gravity = 9.807;
bool Physics::newContacts;

bool Physics::Initialize()
{
	contactManager = new ContactManager();

	collision2DTable[POLYGON_COLLIDER][POLYGON_COLLIDER] = PolygonVsPolygon;
	collision2DTable[POLYGON_COLLIDER][CIRCLE_COLLIDER] = PolygonVsCircle;
	collision2DTable[CIRCLE_COLLIDER][POLYGON_COLLIDER] = CircleVsPolygon;
	collision2DTable[CIRCLE_COLLIDER][CIRCLE_COLLIDER] = CircleVsCircle;

	return true;
}

void Physics::Shutdown()
{
	for (PhysicsObject2D* po : physicsObjects2D)
	{
		Memory::Free(po, sizeof(PhysicsObject2D), MEMORY_TAG_DATA_STRUCT);
	}

	physicsObjects2D.Destroy();

	for (PhysicsObject3D* po : physicsObjects3D)
	{
		Memory::Free(po, sizeof(PhysicsObject3D), MEMORY_TAG_DATA_STRUCT);
	}

	physicsObjects3D.Destroy();

	delete contactManager;
}

void Physics::Update(F64 step)
{
	for (PhysicsObject2D* po : physicsObjects2D)
	{
		PhysicsObject2D& obj = *po;

		if (!obj.kinematic)
		{
			obj.velocity += Vector2::UP * (F32)(gravity * obj.gravityScale * obj.mass * step);
			obj.velocity += obj.force * (F32)obj.massInv;
			obj.velocity += -obj.velocity.Normalized() * obj.velocity.SqrMagnitude() * (F32)(obj.dragCoefficient * obj.area * 0.5 * airDensity * step);

			obj.prevPosition = obj.transform->Position();
			obj.transform->Translate(obj.velocity);
			Vector2 move = obj.velocity + obj.oneTimeVelocity;

			if (obj.collider->type == POLYGON_COLLIDER && !move.IsZero())
			{
				for (Vector2& point : ((PolygonCollider*)obj.collider)->shape) { point += move; }
			}

			if (!move.IsZero())
			{
				newContacts = true;
				contactManager->MoveObject(obj.proxyID, obj.collider->box + obj.prevPosition, move);
			}

			obj.oneTimeVelocity = Vector2::ZERO;
			obj.force = Vector2::ZERO;
		}
	}

	BroadPhase();
	NarrowPhase();
}

PhysicsObject2D* Physics::Create2DPhysicsObject(PhysicsObject2DConfig& config)
{
	static U64 id = 0;

	if (!config.transform)
	{
		Logger::Error("PhysicsObject must have a transform!");
		return nullptr;
	}

	PhysicsObject2D* po = (PhysicsObject2D*)Memory::Allocate(sizeof(PhysicsObject2D), MEMORY_TAG_DATA_STRUCT);
	po->id = id;
	++id;

	switch (config.type)
	{
	case POLYGON_COLLIDER: {
		if (config.shape.Size() < 3)
		{
			Logger::Error("PolygonCollider must have at least 3 sides!");
			Memory::Free(po, sizeof(PhysicsObject2D), MEMORY_TAG_DATA_STRUCT);
			return nullptr;
		}

		PolygonCollider* collider = (PolygonCollider*)Memory::Allocate(sizeof(PolygonCollider), MEMORY_TAG_DATA_STRUCT);
		collider->type = POLYGON_COLLIDER;

		collider->shape = config.shape;
		for (Vector2& point : collider->shape) { point += config.transform->Position(); }

		Box box;

		box.xBounds = { F32_MAX, -F32_MAX };
		box.yBounds = { F32_MAX, -F32_MAX };

		Vector2& lastPoint = config.shape.Back();
		for (Vector2& point : config.shape)
		{
			box.xBounds.x = Math::Min(box.xBounds.x, point.x);
			box.xBounds.y = Math::Max(box.xBounds.y, point.x);
			box.yBounds.x = Math::Min(box.yBounds.x, point.y);
			box.yBounds.y = Math::Max(box.yBounds.y, point.y);

			po->area += (lastPoint.x + point.x) * (lastPoint.y + point.y);
			lastPoint = point;
		}

		collider->box = box;
		po->collider = collider;

		po->area = Math::Abs(po->area * 0.5f);
		po->dragCoefficient = 1.0;
	} break;
	case CIRCLE_COLLIDER: {
		CircleCollider* collider = (CircleCollider*)Memory::Allocate(sizeof(CircleCollider), MEMORY_TAG_DATA_STRUCT);
		collider->type = CIRCLE_COLLIDER;
		collider->radius = config.radius;
		collider->box.xBounds = { config.offset.x - (F32)config.radius, config.offset.x + (F32)config.radius };
		collider->box.yBounds = { config.offset.y - (F32)config.radius, config.offset.y + (F32)config.radius };
		po->collider = collider;
		po->dragCoefficient = 1.17;

		po->area = (F32)(PI * config.radius * config.radius);
	} break;
	default: break;
	}

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

	physicsObjects2D.PushBack(po);

	contactManager->AddObject(po);

	return po;
}

PhysicsObject3D* Physics::Create3DPhysicsObject()
{
	static U64 id = 0;
	PhysicsObject3D* po = (PhysicsObject3D*)Memory::Allocate(sizeof(PhysicsObject3D), MEMORY_TAG_DATA_STRUCT);
	po->id = id;
	++id;

	physicsObjects3D.PushBack(po);

	return po;
}

void Physics::BroadPhase()
{
	if (newContacts)
	{
		contactManager->FindNewContacts();
		newContacts = false;
	}
}

void Physics::NarrowPhase()
{
	List<Contact2D> contacts = contactManager->Contacts();
	for (Contact2D& c : contacts)
	{
		if (collision2DTable[c.a->collider->type][c.b->collider->type](c))
		{
			ResolveCollision(c);
		}
	}
}

bool Physics::CircleVsCircle(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
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
		c.penetration = radius - distance;
		c.normal = direction / distance;
	}
	else
	{
		c.penetration = aCollider->radius;
		c.normal = Vector2::RIGHT;
	}

	return true;
}

bool Physics::PolygonVsPolygon(Contact2D& c)
{
	Vector<Vector2>& aShape = ((PolygonCollider*)c.a->collider)->shape;
	Vector<Vector2>& bShape = ((PolygonCollider*)c.b->collider)->shape;

	List<Vector2> simplex;

	Vector2 direction = c.b->transform->Position() - c.a->transform->Position();

	simplex.PushBack(Support(aShape, bShape, direction));

	direction = -direction;

	while (true)
	{
		simplex.PushBack(Support(aShape, bShape, direction));

		if (simplex.Back().Dot(direction) <= 0.0f) { return false; }
		if (ContainsOrigin(simplex, direction)) { break; }
	}

	while (true)
	{
		Edge e = ClosestEdge(simplex);
		Vector2 p = Support(aShape, bShape, e.normal);
		F32 dist = Math::Abs(e.normal.Dot(p));

		if (dist - e.distance < 0.001f)
		{
			c.normal = e.normal;
			c.penetration = e.distance;
			return true;
		}
		else if (simplex.Size() < aShape.Size() + bShape.Size())
		{
			simplex.Insert(p, e.index);
		}
		else { return false; }
	}
}

bool Physics::PolygonVsCircle(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
	PolygonCollider* aCollider = (PolygonCollider*)a->collider;
	CircleCollider* bCollider = (CircleCollider*)b->collider;
	Vector2 aPos = a->transform->Position();
	Vector2 bPos = b->transform->Position();

	Vector2 closestPoint = bPos + (aPos - bPos).Clamped(aCollider->box.xBounds, aCollider->box.yBounds);
	Vector2 checkX = aCollider->box.xBounds + aPos.x;
	Vector2 checkY = aCollider->box.yBounds + aPos.y;

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

		c.normal = normal.Normalize();
		c.penetration = bCollider->radius - distance;

		return true;
	}

	return false;
}

bool Physics::CircleVsPolygon(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
	CircleCollider* aCollider = (CircleCollider*)a->collider;
	PolygonCollider* bCollider = (PolygonCollider*)b->collider;

	Vector2 n = b->transform->Position() - a->transform->Position();

	F32 xExtent = (bCollider->box.xBounds.y - bCollider->box.xBounds.x) * 0.5f;

	F32 xOverlap = (F32)aCollider->radius + xExtent - Math::Abs(n.x);

	if (xOverlap > 0.0f)
	{
		F32 yExtent = (bCollider->box.yBounds.y - bCollider->box.yBounds.x) * 0.5f;

		F32 yOverlap = (F32)aCollider->radius + yExtent - Math::Abs(n.y);

		if (yOverlap > 0.0f)
		{
			if (yOverlap > xOverlap)
			{
				c.penetration = xOverlap;
				c.normal = n.x < 0.0f ? Vector2::RIGHT : Vector2::LEFT;
			}
			else
			{
				c.penetration = yOverlap;
				c.normal = n.y < 0.0f ? Vector2::DOWN : Vector2::UP;
			}

			return true;
		}
	}

	return false;
}

void Physics::ResolveCollision(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;

	Vector2 relVelocity = b->velocity - a->velocity;

	F32 velAlongNormal = relVelocity.Dot(c.normal);

	if (velAlongNormal > 0.0f || a->massInv + b->massInv <= FLOAT_EPSILON) { return; }

	F64 restitution = Math::Min(a->restitution, b->restitution);
	F64 j = (-(1.0 + restitution) * velAlongNormal) / (a->massInv + b->massInv);

	Vector2 impulse = c.normal * (F32)j;

	a->velocity -= impulse * (F32)a->massInv;
	b->velocity += impulse * (F32)b->massInv;

	F64 slop = 0.001;
	Vector2 correction = c.normal * (F32)(Math::Max(c.penetration - slop, 0.0) / (a->massInv + b->massInv));
	a->oneTimeVelocity -= correction * (F32)a->massInv;
	a->transform->Translate(-correction * (F32)a->massInv);
	b->oneTimeVelocity += correction * (F32)b->massInv;
	b->transform->Translate(correction * (F32)b->massInv);
}

bool Physics::ContainsOrigin(List<Vector2>& simplex, Vector2& direction)
{
	Vector2 a = simplex.Back();
	Vector2 ao = -a;

	if (simplex.Size() == 3)
	{
		Vector2 b = simplex.Front();
		Vector2 c = simplex[1];

		Vector2 ab = b - a;
		Vector2 ac = c - a;

		Vector2 abPerp = TripleProduct(ac, ab, ab);
		Vector2 acPerp = TripleProduct(ab, ac, ac);

		if (abPerp.Dot(ao) > 0.0f)
		{
			simplex.RemoveAt(1);
			direction = abPerp;
		}
		else if (acPerp.Dot(ao) > 0.0f)
		{
			simplex.PopFront();
			direction = acPerp;
		}
		else { return true; }
	}
	else
	{
		Vector2 ab = simplex.Front() - a;
		direction = TripleProduct(ab, ao, ab);
	}

	return false;
}

Vector2 Physics::FarthestPoint(const Vector<Vector2>& shape, const Vector2& direction)
{
	F32 maxDot = -F32_MAX;
	Vector2 farthest;

	for (Vector2& point : shape)
	{
		F32 dot = point.Dot(direction);
		if (dot > maxDot)
		{
			maxDot = dot;
			farthest = point;
		}
	}

	return farthest;
}

Edge Physics::ClosestEdge(const List<Vector2>& simplex)
{
	U32 index = 0;
	Edge closest;
	closest.distance = F32_MAX;

	for (U32 i = 0, j = 1; i < simplex.Size(); ++i, ++j %= simplex.Size())
	{
		const Vector2& a = simplex[i];
		const Vector2& b = simplex[j];

		Vector2 e = b - a;
		Vector2 n = TripleProduct(e, a, e).Normalized();
		F32 d = n.Dot(a);

		if (d < closest.distance && a != b)
		{
			closest.distance = d;
			closest.normal = n;
			closest.index = index;
			closest.vertex0 = a;
			closest.vertex1 = b;
		}
	}

	return closest;
}

Vector2 Physics::Support(const Vector<Vector2>& shape0, const Vector<Vector2>& shape1, const Vector2& direction)
{
	return FarthestPoint(shape0, direction) - FarthestPoint(shape1, -direction);
}

Vector2 Physics::TripleProduct(const Vector2& a, const Vector2& b, const Vector2& c)
{
	F32 z = a.x * b.y - a.y * b.x;
	return { -c.y * z, c.x * z };
}