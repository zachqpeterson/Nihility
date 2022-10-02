#include "Physics.hpp"

#include "Broadphase.hpp"

#include <Containers/Vector.hpp>

List<PhysicsObject2D*> Physics::physicsObjects2D;
List<PhysicsObject3D*> Physics::physicsObjects3D;
Array<Array<Collision2DFn, COLLIDER_2D_MAX>, COLLIDER_2D_MAX> Physics::collision2DTable;

Broadphase* Physics::broadphase;

F32 Physics::airDensity = 1.29f;
F32 Physics::gravity = 9.807f;

bool Physics::Initialize()
{
	//TODO: Set default broadphase

	collision2DTable[BOX_COLLIDER][BOX_COLLIDER] = BoxVsBox;
	collision2DTable[BOX_COLLIDER][CIRCLE_COLLIDER] = BoxVsCircle;
	collision2DTable[BOX_COLLIDER][POLYGON_COLLIDER] = BoxVsPolygon;
	collision2DTable[BOX_COLLIDER][PLATFORM_COLLIDER] = BoxVsPlatform;
	collision2DTable[CIRCLE_COLLIDER][CIRCLE_COLLIDER] = CircleVsCircle;
	collision2DTable[CIRCLE_COLLIDER][BOX_COLLIDER] = CircleVsBox;
	collision2DTable[CIRCLE_COLLIDER][POLYGON_COLLIDER] = CircleVsPolygon;
	collision2DTable[CIRCLE_COLLIDER][PLATFORM_COLLIDER] = CircleVsPlatform;
	collision2DTable[POLYGON_COLLIDER][POLYGON_COLLIDER] = PolygonVsPolygon;
	collision2DTable[POLYGON_COLLIDER][BOX_COLLIDER] = PolygonVsBox;
	collision2DTable[POLYGON_COLLIDER][CIRCLE_COLLIDER] = PolygonVsCircle;
	collision2DTable[POLYGON_COLLIDER][PLATFORM_COLLIDER] = PolygonVsPlatform;
	collision2DTable[PLATFORM_COLLIDER][BOX_COLLIDER] = PlatformVsBox;
	collision2DTable[PLATFORM_COLLIDER][CIRCLE_COLLIDER] = PlatformVsCircle;
	collision2DTable[PLATFORM_COLLIDER][POLYGON_COLLIDER] = PlatformVsPolygon;

	return true;
}

void Physics::SetBroadphase(Broadphase* newBroadphase)
{
	broadphase = newBroadphase;
}

void Physics::Shutdown()
{
	for (PhysicsObject2D* po : physicsObjects2D)
	{
		Memory::Free(po->collider, sizeof(Collider2D), MEMORY_TAG_PHYSICS);
		Memory::Free(po, sizeof(PhysicsObject2D), MEMORY_TAG_PHYSICS);
	}

	physicsObjects2D.Destroy();

	for (PhysicsObject3D* po : physicsObjects3D)
	{
		Memory::Free(po, sizeof(PhysicsObject3D), MEMORY_TAG_PHYSICS);
	}

	physicsObjects3D.Destroy();

	delete broadphase;
}

void Physics::DestroyPhysicsObjects2D(PhysicsObject2D* obj)
{
	physicsObjects2D.Remove(obj);

	Memory::Free(obj->collider, sizeof(Collider2D), MEMORY_TAG_PHYSICS);
	Memory::Free(obj, sizeof(PhysicsObject2D), MEMORY_TAG_PHYSICS);
}

void Physics::Update(F32 step)
{
	for (PhysicsObject2D* po : physicsObjects2D)
	{
		PhysicsObject2D& obj = *po;

		if (!obj.kinematic) //TODO: Check for sleeping
		{
			obj.velocity += Vector2::UP * (gravity * obj.gravityScale * obj.mass * step);
			obj.velocity += obj.force * obj.massInv;
			obj.velocity -= obj.velocity.Normalized() * obj.velocity.SqrMagnitude() * (obj.dragCoefficient * obj.area * 0.5f * airDensity * step);
			if (!obj.freezeRotation)
			{
				obj.angularVelocity += obj.torque * obj.inertiaInv * step;
				obj.angularVelocity -= obj.angularVelocity * obj.angularVelocity * obj.angularDragCoefficient * obj.area * 0.5f * airDensity * step;
			}
		}
		else
		{
			obj.velocity += obj.force * (F32)obj.massInv;
			if (!obj.freezeRotation) { obj.angularVelocity += obj.torque * obj.inertiaInv * step; }
		}

		obj.move = obj.velocity + obj.oneTimeVelocity;

		obj.force = Vector2::ZERO;
		obj.oneTimeVelocity = Vector2::ZERO;
	}

	List<List<Contact2D>> contacts;
	broadphase->Update(contacts);
	NarrowPhase(contacts);

	for (PhysicsObject2D* po : physicsObjects2D)
	{
		PhysicsObject2D& obj = *po;

		obj.move += obj.force + obj.oneTimeVelocity;
		//TODO: if we want to have different gravity directions, this will need to change
		obj.grounded = obj.axisLock.y;
		obj.axisLock = Vector2::ONE * obj.kinematic;

		if (!obj.move.IsZero())
		{
			if (obj.collider->type == POLYGON_COLLIDER)
			{
				for (Vector2& point : ((PolygonCollider*)obj.collider)->shape.vertices) { point += obj.move; }
			}

			broadphase->UpdateObj(po);
			obj.transform->Translate(obj.move);
		}

		obj.velocity += obj.force;

		obj.oneTimeVelocity = Vector2::ZERO;
		obj.force = Vector2::ZERO;
		obj.move = Vector2::ZERO;
	}
}

PhysicsObject2D* Physics::Create2DPhysicsObject(const PhysicsObject2DConfig& config)
{
	static U64 id = 0;

	if (!config.transform)
	{
		Logger::Error("PhysicsObject must have a transform!");
		return nullptr;
	}

	PhysicsObject2D* po = (PhysicsObject2D*)Memory::Allocate(sizeof(PhysicsObject2D), MEMORY_TAG_PHYSICS);
	po->id = id;
	++id;

	switch (config.type)
	{
	case BOX_COLLIDER: {
		BoxCollider* collider = (BoxCollider*)Memory::Allocate(sizeof(BoxCollider), MEMORY_TAG_PHYSICS);
		collider->type = BOX_COLLIDER;
		collider->box = config.box;

		po->collider = collider;

		po->area = config.box.Area();
		po->dragCoefficient = 1.2f;

		po->mass = po->area * config.density;
		if (po->mass == 0.0f)
		{
			po->massInv = 0.0f;
			po->inertia = 0.0f;
			po->inertiaInv = 0.0f;
		}
		else
		{
			po->massInv = 1.0f / po->mass;
			po->inertia = po->mass * collider->box.Extents().Dot(collider->box.Extents()) / 12.0f;
			po->inertiaInv = 1.0f / po->inertia;
		}

	} break;
	case POLYGON_COLLIDER: {
		if (config.shape.Size() < 3)
		{
			Logger::Error("PolygonCollider must have at least 3 sides!");
			Memory::Free(po, sizeof(PhysicsObject2D), MEMORY_TAG_PHYSICS);
			return nullptr;
		}

		PolygonCollider* collider = (PolygonCollider*)Memory::Allocate(sizeof(PolygonCollider), MEMORY_TAG_PHYSICS);
		collider->type = POLYGON_COLLIDER;

		collider->shape.vertices = config.shape;
		//TODO: generate normals
		for (Vector2& point : collider->shape.vertices) { point += config.transform->Position(); }

		Box box;

		box.xBounds = { F32_MAX, -F32_MAX };
		box.yBounds = { F32_MAX, -F32_MAX };

		auto next = config.shape.begin() + 1;
		for (Vector2& point : config.shape)
		{
			Vector2 nextPoint = *next;

			box.xBounds.x = Math::Min(box.xBounds.x, point.x);
			box.xBounds.y = Math::Max(box.xBounds.y, point.x);
			box.yBounds.x = Math::Min(box.yBounds.x, point.y);
			box.yBounds.y = Math::Max(box.yBounds.y, point.y);

			//TODO: Compute area
			collider->shape.normals.Push(point.Normal(nextPoint));
			if (++next == config.shape.end()) { next = config.shape.begin(); }
		}

		collider->box = box;
		po->collider = collider;

		po->area = 1.0;
		po->dragCoefficient = 1.0;
	} break;
	case CIRCLE_COLLIDER: {
		CircleCollider* collider = (CircleCollider*)Memory::Allocate(sizeof(CircleCollider), MEMORY_TAG_PHYSICS);
		collider->type = CIRCLE_COLLIDER;
		collider->radius = config.radius;
		collider->box.xBounds = { config.offset.x - (F32)config.radius, config.offset.x + (F32)config.radius };
		collider->box.yBounds = { config.offset.y - (F32)config.radius, config.offset.y + (F32)config.radius };
		po->collider = collider;
		po->dragCoefficient = 1.17f;

		po->area = (F32)(PI * config.radius * config.radius);
	} break;
	default: break;
	}

	po->collider->trigger = config.trigger;
	po->transform = config.transform;

	po->restitution = config.restitution;
	po->friction = config.restitution;
	po->gravityScale = config.gravityScale;
	po->kinematic = config.kinematic;
	po->freezeRotation = config.freezeRotation;

	po->layerMask = config.layerMask;

	physicsObjects2D.PushBack(po);
	broadphase->InsertObj(po);

	return po;
}

PhysicsObject3D* Physics::Create3DPhysicsObject()
{
	static U64 id = 0;
	PhysicsObject3D* po = (PhysicsObject3D*)Memory::Allocate(sizeof(PhysicsObject3D), MEMORY_TAG_PHYSICS);
	po->id = id;
	++id;

	physicsObjects3D.PushBack(po);

	return po;
}

void Physics::NarrowPhase(List<List<Contact2D>>& contacts)
{
	for (List<Contact2D>& contact : contacts)
	{
		ResolveCollisions(contact);
	}

	//for (PhysicsObject2D* obj0 : physicsObjects2D)
	//for (auto it0 = physicsObjects2D.begin(); it0 != physicsObjects2D.end(); ++it0)
	//{
	//	//Vector<PhysicsObject2D*> result;
	//	//tree->Query(obj0, result);
	//	//
	//	//for (PhysicsObject2D* obj1 : result)
	//	//{
	//	//	U32 id0 = obj0->id;
	//	//	U32 id1 = obj1->id;
	//	//
	//	//	if (id0 > id1) { Math::Swap(id0, id1); }
	//	//
	//	//	if (!table.GetSet(id0, id1))
	//	//	{
	//	//		Contact2D c = { obj0, obj1 };
	//	//
	//	//		if (!c.a->kinematic && !c.b->kinematic) { dynamics.PushBack(c); }
	//	//		else if (collision2DTable[c.a->collider->type][c.b->collider->type](c)) { ResolveCollision(c); }
	//	//	}
	//	//}
	//
	//	for (auto it1 = it0 + 1; it1 != physicsObjects2D.end(); ++it1)
	//	{
	//		U64 id0 = (*it0)->id;
	//		U64 id1 = (*it1)->id;
	//		if (id0 > id1) { Math::Swap(id0, id1); }
	//		Contact2D c = { *it0, *it1 };
	//
	//		if (c.a->layerMask & c.b->layerMask && (!c.a->kinematic || !c.b->kinematic))
	//		{
	//			if (!c.a->kinematic && !c.b->kinematic) { dynamics.PushBack(c); }
	//			else if (collision2DTable[c.a->collider->type][c.b->collider->type](c)) { ResolveCollision(c); }
	//		}
	//	}
	//}
	//
	//for (Contact2D& c : dynamics)
	//{
	//	if (collision2DTable[c.a->collider->type][c.b->collider->type](c))
	//	{d
	//		ResolveCollision(c);
	//	}
	//}
}

void Physics::ResolveCollisions(List<Contact2D>& contacts)
{
	for (Contact2D& c : contacts)
	{
		PhysicsObject2D* a = c.a;
		PhysicsObject2D* b = c.b;

		if (!b)
		{
			Vector2 onlyMove = c.a->move - c.a->velocity;

			F32 relVelNorm = c.normal.Dot(c.relativeVelocity - onlyMove);
			F32 relVelNormMove = c.normal.Dot(onlyMove);
			Vector2 impulse = c.normal * (-(1.0f + c.restitution) * relVelNorm);
			Vector2 impulseMove = c.normal * (-(1.0f + c.restitution) * relVelNormMove);
			a->force += impulse * !a->axisLock;
			a->oneTimeVelocity += impulseMove * !a->axisLock;

			F32 percent = 0.9999f;
			Vector2 correction = c.normal * (c.distance * percent);
			a->oneTimeVelocity += correction * !a->axisLock;

			bool lock = c.restitution < FLOAT_EPSILON;
			Vector2 relVel = (a->move + a->force);

			a->axisLock += { (F32)((c.normal.x > 0.0f)* lock* Math::Zero(relVel.x)), (F32)((c.normal.y > 0.0f)* lock* Math::Zero(relVel.y)) };
		}
		else
		{
			F32 relVelNorm = c.normal.Dot(c.relativeVelocity);

			Vector2 massRatio = !a->axisLock * a->massInv + !b->axisLock * b->massInv;
			massRatio.x += Math::Zero(massRatio.x);
			massRatio.y += Math::Zero(massRatio.y);
			Vector2 impulse = c.normal * (-(1.0f + c.restitution) * relVelNorm) / massRatio;

			a->force += impulse * (!a->axisLock * a->massInv);
			b->force -= impulse * (!b->axisLock * b->massInv);

			bool lock = c.restitution < FLOAT_EPSILON;
			Vector2 relVel = (a->move + a->force) - (b->move + b->force);

			F32 percent = 0.9999f;
			Vector2 correction = c.normal * (c.penetration * percent) / massRatio;
			a->oneTimeVelocity += correction * Vector2{ (F32)(!a->axisLock.x * Math::Zero(relVel.x)), (F32)(!a->axisLock.y * Math::Zero(relVel.y)) } *a->massInv;
			b->oneTimeVelocity -= correction * Vector2{ (F32)(!b->axisLock.x * Math::Zero(relVel.x)), (F32)(!b->axisLock.y * Math::Zero(relVel.y)) } *b->massInv;

			a->axisLock += { b->axisLock.x* (c.normal.x > 0.0f)* lock* Math::Zero(relVel.x),
				b->axisLock.y* (c.normal.y > 0.0f)* lock* Math::Zero(relVel.y) };
			b->axisLock += { a->axisLock.x* (c.normal.x < 0.0f)* lock* Math::Zero(relVel.x),
				a->axisLock.y* (c.normal.y < 0.0f)* lock* Math::Zero(relVel.y)};
		}
	}
}

bool Physics::BoxVsBox(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
	Box& aBox = a->collider->box;
	Box& bBox = b->collider->box;

	c.relativeVelocity = a->move * !a->axisLock - b->move * !b->axisLock;
	Vector2 direction = a->transform->Position() - b->transform->Position();
	if (a->massInv + b->massInv < FLOAT_EPSILON || c.relativeVelocity.Dot(direction) > FLOAT_EPSILON) { return false; }

	Box mink = (bBox + b->transform->Position()).Minkowski(aBox + a->transform->Position());
	F32 t;

	if (mink.Contains(Vector2::ZERO))
	{
		if (c.relativeVelocity.IsZero())
		{
			Vector2 direction = mink.DirectionToClosestBound(Vector2::ZERO);
			c.penetration = -direction.Magnitude();
			c.normal = direction / c.penetration;
			c.restitution = Math::Min(a->restitution, b->restitution);
		}
		else
		{
			//TODO: This assumption may cause problems
			//TODO: It did, if it's not going to hit on one axis, that axis should be zero on the normal
			//TODO: In other words, be able to slide instead on bound back perfectly on the normal
			//TODO: This will need to be implemented on other cases as well
			c.normal = c.relativeVelocity.Normalized();

			F32 x = (mink.xBounds.x * c.normal.x * (c.normal.x > 0.0f)) + (mink.xBounds.y * c.normal.x * (c.normal.x < 0.0f));
			F32 y = (mink.yBounds.x * c.normal.y * (c.normal.y > 0.0f)) + (mink.yBounds.y * c.normal.y * (c.normal.y < 0.0f));

			c.penetration = -Math::Sqrt(x * x + y * y);
			c.restitution = Math::Min(a->restitution, b->restitution);
		}

		return true;
	}
	else if ((t = mink.TOI(Vector2::ZERO, c.relativeVelocity)) < F32_MAX && t > 0.0f)
	{
		Vector2 direction = c.relativeVelocity * t;
		c.penetration = direction.Magnitude();
		c.normal = direction / c.penetration;
		c.restitution = Math::Min(a->restitution, b->restitution);

		return true;
	}

	return false;
}

bool Physics::BoxVsCircle(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
	Box& aBox = a->collider->box;
	CircleCollider* bCircle = (CircleCollider*)b->collider;

	return false;
}

bool Physics::BoxVsPolygon(Contact2D& c)
{
	return false;
}

bool Physics::BoxVsPlatform(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
	Box& aBox = a->collider->box;
	PlatformCollider* bPlatform = (PlatformCollider*)b->collider;

	if (a->passThrough) { return false; }

	return false;
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
		//c.penetration = radius - distance;
		//c.normal = direction / distance;
	}
	else
	{
		//c.penetration = aCollider->radius;
		//c.normal = Vector2::RIGHT;
	}

	return true;
}

bool Physics::CircleVsBox(Contact2D& c)
{
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
				//c.penetration = xOverlap;
				//c.normal = n.x < 0.0f ? Vector2::RIGHT : Vector2::LEFT;
			}
			else
			{
				//c.penetration = yOverlap;
				//c.normal = n.y < 0.0f ? Vector2::DOWN : Vector2::UP;
			}

			return true;
		}
	}

	return false;
}

bool Physics::CircleVsPlatform(Contact2D& c)
{
	return false;
}

bool Physics::PolygonVsPolygon(Contact2D& c)
{
	Shape& aShape = ((PolygonCollider*)c.a->collider)->shape;
	Shape& bShape = ((PolygonCollider*)c.b->collider)->shape;

	List<Vector2> simplex;

	Vector2 direction = c.b->transform->Position() - c.a->transform->Position();

	simplex.PushBack(FindSupport(aShape.vertices, bShape.vertices, direction));

	direction = -direction;

	while (true)
	{
		simplex.PushBack(FindSupport(aShape.vertices, bShape.vertices, direction));

		if (simplex.Back().Dot(direction) <= 0.0f) { return false; }
		if (ContainsOrigin(simplex, direction)) { break; }
	}

	while (true)
	{
		Edge e = ClosestEdge(simplex);
		Vector2 p = FindSupport(aShape.vertices, bShape.vertices, e.normal);
		F32 dist = Math::Abs(e.normal.Dot(p));

		if (dist - e.distance < 0.001f)
		{
			//c.normal = e.normal;
			//c.penetration = e.distance;
			return true;
		}
		else if (simplex.Size() < aShape.vertices.Size() + bShape.vertices.Size())
		{
			simplex.Insert(p, e.index);
		}
		else { return false; }
	}
}

bool Physics::PolygonVsBox(Contact2D& c)
{
	return false;
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

		//c.normal = normal.Normalize();
		//c.penetration = bCollider->radius - distance;

		return true;
	}

	return false;
}

bool Physics::PolygonVsPlatform(Contact2D& c)
{
	return false;
}

bool Physics::PlatformVsBox(Contact2D& c)
{
	PhysicsObject2D* a = c.a;
	PhysicsObject2D* b = c.b;
	PlatformCollider* aPlatform = (PlatformCollider*)a->collider;
	Box& bBox = b->collider->box;

	if (b->passThrough) { return false; }

	//basically box vs box but only collide on the top
	return false;
}

bool Physics::PlatformVsCircle(Contact2D& c)
{
	return false;
}

bool Physics::PlatformVsPolygon(Contact2D& c)
{
	return false;
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

Vector2 Physics::FindSupport(const Vector<Vector2>& shape0, const Vector<Vector2>& shape1, const Vector2& direction)
{
	return FarthestPoint(shape0, direction) - FarthestPoint(shape1, -direction);
}

Vector2 Physics::TripleProduct(const Vector2& a, const Vector2& b, const Vector2& c)
{
	F32 z = a.x * b.y - a.y * b.x;
	return { -c.y * z, c.x * z };
}

F32 Physics::TOI(const Vector2& p, const Vector2& endP, const Vector2& q, const Vector2& endQ)
{
	Vector2 r = endP - p;
	Vector2 s = endQ - q;

	F32 det = 1.0f / (-s.x * r.y + r.x * s.y);

	if (Math::Inf(det)) { return F32_MAX; }

	F32 u = (-r.y * (p.x - q.x) + r.x * (p.y - q.y)) * det;
	F32 t = (s.x * (p.y - q.y) - s.y * (p.x - q.x)) * det;

	if ((t >= 0) && (t <= 1) && (u >= 0) && (u <= 1)) { return t; }

	return F32_MAX;
}

F32 Box::TOI(const Vector2& origin, const Vector2& direction) const
{
	Vector2 end = origin + direction;

	F32 minT = Physics::TOI(origin, end, { xBounds.x, yBounds.x }, { xBounds.x, yBounds.y });
	F32 x = Physics::TOI(origin, end, { xBounds.x, yBounds.y }, { xBounds.y, yBounds.y });

	minT = Math::Min(x, minT);
	x = Physics::TOI(origin, end, { xBounds.y, yBounds.y }, { xBounds.y, yBounds.x });
	minT = Math::Min(x, minT);
	x = Physics::TOI(origin, end, { xBounds.y, yBounds.x }, { xBounds.x, yBounds.x });

	return Math::Min(x, minT);
}
