#include "Physics.hpp"

#include "ContactManager.hpp"

#include <Containers/Vector.hpp>

//#include "Core/Time.hpp"

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
				for (Vector2& point : ((PolygonCollider*)obj.collider)->shape.vertices) { point += move; }
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
		//c.penetration = radius - distance;
		c.normal = direction / distance;
	}
	else
	{
		//c.penetration = aCollider->radius;
		c.normal = Vector2::RIGHT;
	}

	return true;
}

bool Physics::PolygonVsPolygon(Contact2D& c)
{
	Shape& aShape = ((PolygonCollider*)c.a->collider)->shape;
	Shape& bShape = ((PolygonCollider*)c.b->collider)->shape;

	if (GJK(c) < FLOAT_EPSILON)
	{
		c.count = 0;
		I32 ea, eb;
		F32 sa, sb;
		if ((sa = CheckFaces(aShape, bShape, ea)) >= 0) { return false; }
		if ((sb = CheckFaces(bShape, aShape, eb)) >= 0) { return false; }

		const Shape* rp, * ip;
		I32 re;
		F32 kRelTol = 0.95f, kAbsTol = 0.01f;
		bool flip;
		if (sa * kRelTol > sb + kAbsTol)
		{
			rp = &aShape;
			ip = &bShape;
			re = ea;
			flip = false;
		}
		else
		{
			rp = &bShape;
			ip = &aShape;
			re = eb;
			flip = true;
		}

		Vector2 incident[2];
		Incident(incident, ip, rp, re);
		HalfSpace rh;
		if (!SidePlanes(incident, rp, re, rh)) { return false; }
		KeepDeep(incident, rh, c);
		if (flip) { c.normal = -c.normal; }

		return true;
	}

	return false;

	//----------------OLD----------------

	/*List<Vector2> simplex;

	Vector2 direction = c.b->transform->Position() - c.a->transform->Position();

	simplex.PushBack(FindSupport(aShape, bShape, direction));

	direction = -direction;

	while (true)
	{
		simplex.PushBack(FindSupport(aShape, bShape, direction));

		if (simplex.Back().Dot(direction) <= 0.0f) { return false; }
		if (ContainsOrigin(simplex, direction)) { break; }
	}

	while (true)
	{
		Edge e = ClosestEdge(simplex);
		Vector2 p = FindSupport(aShape, bShape, e.normal);
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
	}*/
}

F32 Physics::GJK(Contact2D& c)
{
	Shape& aShape = ((PolygonCollider*)c.a->collider)->shape;
	Shape& bShape = ((PolygonCollider*)c.b->collider)->shape;

	Simplex s;
	Support* verts = &s.a;
	//TODO: Don't do this with cached info
	s.a.iA = 0;
	s.a.iB = 0;
	s.a.sA = aShape.vertices.Front();
	s.a.sB = bShape.vertices.Front();
	s.a.p = s.a.sB - s.a.sA;
	s.a.u = 1.0f;
	s.div = 1.0f;
	s.count = 1;

	I32 saveA[3], saveB[3];
	I32 save_count = 0;
	F32 d0 = F32_MAX;
	F32 d1 = F32_MAX;
	I32 iter = 0;
	I32 hit = 0;

	bool dup = false;
	while (iter < 20 && !dup)
	{
		save_count = s.count;
		for (I32 i = 0; i < save_count; ++i)
		{
			saveA[i] = verts[i].iA;
			saveB[i] = verts[i].iB;
		}

		switch (save_count)
		{
		case 3: TriangleCase(s); break;
		case 2: LineCase(s); break;
		case 1:	
		default: break;
		}

		if (s.count == 3)
		{
			hit = 1;
			break;
		}

		Vector2 p = GetDistance(s);
		d1 = p.SqrMagnitude();

		if (d1 > d0) { break; }
		d0 = d1;

		Vector2 d = GetDirection(s);
		if (d.SqrMagnitude() < FLOAT_EPSILON * FLOAT_EPSILON) { break; }

		//TODO: Apply rotation to direction
		I32 iA = GetSupport(aShape.vertices, -d);
		Vector2 sA = aShape.vertices[iA];
		I32 iB = GetSupport(bShape.vertices, d);
		Vector2 sB = bShape.vertices[iB];

		++iter;

		for (int i = 0; i < save_count && !dup; ++i)
		{
			dup = iA == saveA[i] && iB == saveB[i];
		}

		Support* v = verts + s.count;
		v->iA = iA;
		v->sA = sA;
		v->iB = iB;
		v->sB = sB;
		v->p = v->sB - v->sA;
		++s.count;
	}

	Vector2 a, b;
	Witness(s, a, b);
	F32 dist = (a - b).Magnitude();

	if (hit)
	{
		a = b;
		dist = 0;
	}

	if(dist <= FLOAT_EPSILON)
	{
		Vector2 p = (a + b) * 0.5f;
		a = p;
		b = p;
		dist = 0;
	}

	//TODO: cache stuff

	return dist;
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
		//c.penetration = bCollider->radius - distance;

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
				//c.penetration = xOverlap;
				c.normal = n.x < 0.0f ? Vector2::RIGHT : Vector2::LEFT;
			}
			else
			{
				//c.penetration = yOverlap;
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
	c.depths;
	//Vector2 correction = c.normal * (F32)(Math::Max(c.penetration - slop, 0.0) / (a->massInv + b->massInv));
	//a->oneTimeVelocity -= correction * (F32)a->massInv;
	//a->transform->Translate(-correction * (F32)a->massInv);
	//b->oneTimeVelocity += correction * (F32)b->massInv;
	//b->transform->Translate(correction * (F32)b->massInv);
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

void Physics::LineCase(Simplex& s)
{
	Vector2 a = s.a.p;
	Vector2 b = s.b.p;
	F32 u = b.Dot((b - a).Normalized());
	F32 v = a.Dot((a - b).Normalized());

	if (v <= 0)
	{
		s.a.u = 1.0f;
		s.div = 1.0f;
		s.count = 1;
	}
	else if (u <= 0)
	{
		s.a = s.b;
		s.a.u = 1.0f;
		s.div = 1.0f;
		s.count = 1;
	}
	else
	{
		s.a.u = u;
		s.b.u = v;
		s.div = u + v;
		s.count = 2;
	}
}

void Physics::TriangleCase(Simplex& s)
{
	Vector2 a = s.a.p;
	Vector2 b = s.b.p;
	Vector2 c = s.c.p;

	F32 uAB = b.Dot((b - a).Normalized());
	F32 vAB = a.Dot((a - b).Normalized());
	F32 uBC = c.Dot((c - b).Normalized());
	F32 vBC = b.Dot((b - c).Normalized());
	F32 uCA = a.Dot((a - c).Normalized());
	F32 vCA = c.Dot((c - a).Normalized());
	F32 area = (b - a).Normalized().Determinant((c - a).Normalized());
	F32 uABC = b.Determinant(c) * area;
	F32 vABC = c.Determinant(a) * area;
	F32 wABC = a.Determinant(b) * area;

	if (vAB <= 0 && uCA <= 0)
	{
		s.a.u = 1.0f;
		s.div = 1.0f;
		s.count = 1;
	}
	else if (uAB <= 0 && vBC <= 0)
	{
		s.a = s.b;
		s.a.u = 1.0f;
		s.div = 1.0f;
		s.count = 1;
	}
	else if (uBC <= 0 && vCA <= 0)
	{
		s.a = s.c;
		s.a.u = 1.0f;
		s.div = 1.0f;
		s.count = 1;
	}
	else if (uAB > 0 && vAB > 0 && wABC <= 0)
	{
		s.a.u = uAB;
		s.b.u = vAB;
		s.div = uAB + vAB;
		s.count = 2;
	}
	else if (uBC > 0 && vBC > 0 && uABC <= 0)
	{
		s.a = s.b;
		s.b = s.c;
		s.a.u = uBC;
		s.b.u = vBC;
		s.div = uBC + vBC;
		s.count = 2;
	}
	else if (uCA > 0 && vCA > 0 && vABC <= 0)
	{
		s.b = s.a;
		s.a = s.c;
		s.a.u = uCA;
		s.b.u = vCA;
		s.div = uCA + vCA;
		s.count = 2;
	}
	else
	{
		s.a.u = uABC;
		s.b.u = vABC;
		s.c.u = wABC;
		s.div = uABC + vABC + wABC;
		s.count = 3;
	}
}

Vector2 Physics::GetDistance(const Simplex& s)
{
	F32 den = 1.0f / s.div;
	switch (s.count)
	{
	case 1: return s.a.p;
	case 2: return (s.a.p * (den * s.a.u)) + (s.b.p * (den * s.b.u));
	case 3:	return (s.a.p * (den * s.a.u)) + (s.b.p * (den * s.b.u)) + (s.c.p * (den * s.c.u));
	default: return Vector2::ZERO;
	}
}

Vector2 Physics::GetDirection(const Simplex& s)
{
	switch (s.count)
	{
	case 1: return -s.a.p;
	case 2:
	{
		Vector2 ab = s.b.p - s.a.p;
		if (ab.Determinant(-s.a.p) > 0.0f) { return ab.Skewed(); }
		return ab.Skewed90();
	}
	case 3:
	default: return Vector2::ZERO;
	}
}

void Physics::Witness(const Simplex& s, Vector2& a, Vector2& b)
{
	F32 den = 1.0f / s.div;

	switch (s.count)
	{
	case 1:	a = s.a.sA;
			b = s.a.sB; break;
	case 2:	a = (s.a.sA * (den * s.a.u)) + (s.b.sA * (den * s.b.u)); 
			b = (s.a.sB * (den * s.a.u)) + (s.b.sB * (den * s.b.u)); break;
	case 3:	a = (s.a.sA * (den * s.a.u)) + (s.b.sA * (den * s.b.u)) + (s.c.sA * (den * s.c.u));	
			b = (s.a.sB * (den * s.a.u)) + (s.b.sB * (den * s.b.u)) + (s.c.sB * (den * s.c.u)); break;
	default:a = Vector2::ZERO;
			b = Vector2::ZERO; break;
	}
}

U32 Physics::GetSupport(const Vector<Vector2>& verts, const Vector2& d)
{
	U32 imax = 0;
	F32 dmax = verts[0].Dot(d);

	for (U32 i = 1; i < verts.Size(); ++i)
	{
		F32 dot = verts[i].Dot(d);
		if (dot > dmax)
		{
			imax = i;
			dmax = dot;
		}
	}

	return imax;
}

F32 Physics::CheckFaces(const Shape& a, const Shape& b, I32& faceIndex)
{
	F32 separation = -F32_MAX;
	I32 index = ~0;

	for (I32 i = 0; i < a.vertices.Size(); ++i)
	{
		HalfSpace h = PlaneAt(a, i);
		I32 idx = GetSupport(b.vertices, -h.normal);
		Vector2 p = b.vertices[idx];
		F32 d = h.Distance(p);
		if (d > separation)
		{
			separation = d;
			index = i;
		}
	}

	faceIndex = index;
	return separation;
}

HalfSpace Physics::PlaneAt(const Shape& s, I32 i)
{
	HalfSpace h;
	h.normal = s.normals[i];
	h.distance = s.normals[i].Dot(s.vertices[i]);
	return h;
}

void Physics::Incident(Vector2* incident, const Shape* ip, const Shape* rp, I32 re)
{
	I32 index = ~0;
	F32 minDot = F32_MAX;
	for (int i = 0; i < ip->vertices.Size(); ++i)
	{
		F32 dot = rp->normals[re].Dot(ip->normals[i]);
		if (dot < minDot)
		{
			minDot = dot;
			index = i;
		}
	}

	incident[0] = ip->vertices[index];
	incident[1] = ip->vertices[index + 1 == ip->vertices.Size() ? 0 : index + 1];
}

I32 Physics::SidePlanes(Vector2* seg, const Shape* p, I32 e, HalfSpace& h)
{
	Vector2 ra = p->vertices[e];
	Vector2 rb = p->vertices[e + 1 == p->vertices.Size() ? 0 : e + 1];
	Vector2 in = (rb - ra).Normalized();
	HalfSpace left = { -in, ra.Dot(-in) };
	HalfSpace right = { in, in.Dot(rb) };

	if (Clip(seg, left) < 2 || Clip(seg, right) < 2) { return 0; }

	h.normal = in.Skew90();
	h.distance = in.Dot(ra);
	return 1;
}

I32 Physics::Clip(Vector2* seg, const HalfSpace& h)
{
	Vector2 out[2];
	I32 sp = 0;
	F32 d0, d1;
	if ((d0 = h.Distance(seg[0])) < 0) { out[sp++] = seg[0]; }
	if ((d1 = h.Distance(seg[1])) < 0) { out[sp++] = seg[1]; }
	if (d0 * d1 <= 0) { out[sp++] = seg[0] + (seg[1] - seg[0]) * (d0 / (d0 - d1)); } //Intersect
	seg[0] = out[0]; seg[1] = out[1];
	return sp;
}

void Physics::KeepDeep(Vector2* seg, HalfSpace h, Contact2D& c)
{
	I32 cp = 0;
	for (I32 i = 0; i < 2; ++i)
	{
		Vector2 p = seg[i];
		F32 d = h.Distance(p);
		if (d < 0)
		{
			c.contactPoints[cp] = p;
			c.depths[cp] = -d;
			++cp;
		}
	}
	c.count = cp;
	c.normal = -h.normal;
}