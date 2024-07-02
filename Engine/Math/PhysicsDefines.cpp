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

#include "PhysicsDefines.hpp"

#include "Physics.hpp"
#include "Broadphase.hpp"
#include "Resources\Scene.hpp"

void RigidBody2D::Update(Scene* scene)
{
	Entity* e = scene->GetEntity(entityID);
	e->transform.SetRotation(Vector3{ 0.0f, 0.0f, sweep.a });
	e->transform.SetPosition((Vector3)(sweep.c - sweep.localCenter * Quaternion2{ sweep.a }));
}

void RigidBody2D::Load(Scene* scene) {}

void RigidBody2D::Cleanup(Scene* scene)
{
	colliders.Cleanup();
	contacts.Cleanup();
}

void RigidBody2D::UpdateTransform()
{
	sweep.c = sweep.localCenter * transform;
	sweep.a = transform.rotation.Angle();

	sweep.c0 = sweep.c;
	sweep.a0 = sweep.a;

	for (Collider2D& c : colliders)
	{
		Physics::Synchronize(c, transform, transform);
	}
}

void RigidBody2D::SynchronizeFixtures()
{
	Transform2D xf1;
	xf1.rotation = sweep.a0;
	xf1.position = sweep.c0 - sweep.localCenter * xf1.rotation;

	for (Collider2D& c : colliders)
	{
		Physics::Synchronize(c, xf1, transform);
	}
}

void RigidBody2D::AddCollider(const Collider2D& collider)
{
	Collider2D& c = colliders.Push(collider);
	c.body = this;

	if (flags & FLAG_ACTIVE)
	{
		c.proxy.aabb = c.ComputeAABB(transform);
		c.proxy.proxyId = Broadphase::CreateProxy(c.proxy.aabb, &c.proxy);
		c.proxy.collider = &c;
	}

	ResetMassData();

	Physics::flags |= Physics::FLAG_NEW_FIXTURE;
}

void RigidBody2D::SetActive(bool b)
{
	if (b == ((flags & FLAG_ACTIVE) != 0)) { return; }

	if (b)
	{
		flags |= FLAG_ACTIVE;

		for (Collider2D& c : colliders)
		{
			c.proxy.aabb = c.ComputeAABB(transform);
			c.proxy.proxyId = Broadphase::CreateProxy(c.proxy.aabb, &c.proxy);
			c.proxy.collider = &c;
		}
	}
	else
	{
		flags &= ~FLAG_ACTIVE;

		for (Collider2D& c : colliders)
		{
			Broadphase::DestroyProxy(c.proxy.proxyId);
			c.proxy.proxyId = U32_MAX;
		}

		for (ContactEdge2D& c : contacts)
		{
			Physics::contactFreelist.Release(c.contact->index);
			c.contact->valid = false;
			c.contact->index = U32_MAX;
		}

		contacts.Clear();
	}
}

void RigidBody2D::SetBodyType(BodyType type)
{
	if (this->type == type) { return; }

	this->type = type;

	ResetMassData();

	if (type == BODY_TYPE_STATIC)
	{
		velocity = Vector2Zero;
		angularVelocity = 0.0f;
		sweep.a0 = sweep.a;
		sweep.c0 = sweep.c;
		SynchronizeFixtures();
	}

	SetAwake(true);

	force = Vector2Zero;
	torque = 0.0f;

	for (ContactEdge2D& c : contacts)
	{
		Physics::contactFreelist.Release(c.contact->index);
		c.contact->valid = false;
		c.contact->index = U32_MAX;
	}

	contacts.Clear();

	// Touch the proxies so that new contacts will be created (when appropriate)
	for (Collider2D& c : colliders)
	{
		Broadphase::TouchProxy(c.proxy.proxyId);
	}
}

void Simplex::ReadCache(const SimplexCache* cache,
	const DistanceProxy* proxyA, const Transform2D& transformA,
	const DistanceProxy* proxyB, const Transform2D& transformB)
{
	// Copy data from cache.
	count = cache->count;
	SimplexVertex* vertices = &v1;
	for (U32 i = 0; i < count; ++i)
	{
		SimplexVertex* v = vertices + i;
		v->indexA = cache->indexA[i];
		v->indexB = cache->indexB[i];
		Vector2 wALocal = proxyA->vertices[v->indexA];
		Vector2 wBLocal = proxyB->vertices[v->indexB];
		v->wA = wALocal * transformA;
		v->wB = wBLocal * transformB;
		v->w = v->wB - v->wA;
		v->a = 0.0f;
	}

	// Compute the new simplex metric, if it is substantially different than
	// old metric then flush the simplex.
	if (count > 1)
	{
		F32 metric1 = cache->metric;
		F32 metric2 = GetMetric();
		if (metric2 < 0.5f * metric1 || 2.0f * metric1 < metric2 || metric2 < Traits<F32>::Epsilon)
		{
			// Reset the simplex.
			count = 0;
		}
	}

	// If the cache is empty or invalid ...
	if (count == 0)
	{
		SimplexVertex* v = vertices + 0;
		v->indexA = 0;
		v->indexB = 0;
		Vector2 wALocal = proxyA->vertices[0];
		Vector2 wBLocal = proxyB->vertices[0];
		v->wA = wALocal * transformA;
		v->wB = wBLocal * transformB;
		v->w = v->wB - v->wA;
		v->a = 1.0f;
		count = 1;
	}
}

void Simplex::WriteCache(SimplexCache* cache) const
{
	cache->metric = GetMetric();
	cache->count = (U16)count;
	const SimplexVertex* vertices = &v1;
	for (U32 i = 0; i < count; ++i)
	{
		cache->indexA[i] = (U8)vertices[i].indexA;
		cache->indexB[i] = (U8)vertices[i].indexB;
	}
}

Vector2 Simplex::GetSearchDirection() const
{
	switch (count)
	{
	case 1: return -v1.w;
	case 2: {
		Vector2 e12 = v2.w - v1.w;
		F32 sgn = e12.Cross(-v1.w);
		if (sgn > 0.0f) { return e12.CrossInv(1.0f); }
		else { return e12.Cross(1.0f); }
	} break;
	default: return Vector2Zero;
	}
}

Vector2 Simplex::GetClosestPoint() const
{
	switch (count)
	{
	case 0: return Vector2Zero;
	case 1: return v1.w;
	case 2: return v1.a * v1.w + v2.a * v2.w;
	case 3: return Vector2Zero;
	default: return Vector2Zero;
	}
}

void Simplex::GetWitnessPoints(Vector2* pA, Vector2* pB) const
{
	switch (count)
	{
	case 0: break;
	case 1: {
		*pA = v1.wA;
		*pB = v1.wB;
	} break;
	case 2: {
		*pA = v1.a * v1.wA + v2.a * v2.wA;
		*pB = v1.a * v1.wB + v2.a * v2.wB;
	} break;
	case 3: {
		*pA = v1.a * v1.wA + v2.a * v2.wA + v3.a * v3.wA;
		*pB = *pA;
	} break;
	default: break;
	}
}

F32 Simplex::GetMetric() const
{
	switch (count)
	{
	case 0: return 0.0f;
	case 1: return 0.0f;
	case 2: return (v1.w - v2.w).Magnitude();
	case 3: return (v2.w - v1.w).Cross(v3.w - v1.w);
	default: return 0.0f;
	}
}

void Simplex::Solve2()
{
	Vector2 w1 = v1.w;
	Vector2 w2 = v2.w;
	Vector2 e12 = w2 - w1;

	F32 d12_2 = -w1.Dot(e12);
	if (d12_2 <= 0.0f)
	{
		v1.a = 1.0f;
		count = 1;
		return;
	}

	F32 d12_1 = w2.Dot(e12);
	if (d12_1 <= 0.0f)
	{
		v2.a = 1.0f;
		count = 1;
		v1 = v2;
		return;
	}

	F32 inv_d12 = 1.0f / (d12_1 + d12_2);
	v1.a = d12_1 * inv_d12;
	v2.a = d12_2 * inv_d12;
	count = 2;
}

void Simplex::Solve3()
{
	Vector2 w1 = v1.w;
	Vector2 w2 = v2.w;
	Vector2 w3 = v3.w;

	Vector2 e12 = w2 - w1;
	F32 w1e12 = w1.Dot(e12);
	F32 w2e12 = w2.Dot(e12);
	F32 d12_1 = w2e12;
	F32 d12_2 = -w1e12;

	Vector2 e13 = w3 - w1;
	F32 w1e13 = w1.Dot(e13);
	F32 w3e13 = w3.Dot(e13);
	F32 d13_1 = w3e13;
	F32 d13_2 = -w1e13;

	Vector2 e23 = w3 - w2;
	F32 w2e23 = w2.Dot(e23);
	F32 w3e23 = w3.Dot(e23);
	F32 d23_1 = w3e23;
	F32 d23_2 = -w2e23;

	F32 n123 = e12.Cross(e13);

	F32 d123_1 = n123 * w2.Cross(w3);
	F32 d123_2 = n123 * w3.Cross(w1);
	F32 d123_3 = n123 * w1.Cross(w2);

	if (d12_2 <= 0.0f && d13_2 <= 0.0f)
	{
		v1.a = 1.0f;
		count = 1;
		return;
	}

	if (d12_1 > 0.0f && d12_2 > 0.0f && d123_3 <= 0.0f)
	{
		F32 inv_d12 = 1.0f / (d12_1 + d12_2);
		v1.a = d12_1 * inv_d12;
		v2.a = d12_2 * inv_d12;
		count = 2;
		return;
	}

	if (d13_1 > 0.0f && d13_2 > 0.0f && d123_2 <= 0.0f)
	{
		F32 inv_d13 = 1.0f / (d13_1 + d13_2);
		v1.a = d13_1 * inv_d13;
		v3.a = d13_2 * inv_d13;
		count = 2;
		v2 = v3;
		return;
	}

	if (d12_1 <= 0.0f && d23_2 <= 0.0f)
	{
		v2.a = 1.0f;
		count = 1;
		v1 = v2;
		return;
	}

	if (d13_1 <= 0.0f && d23_1 <= 0.0f)
	{
		v3.a = 1.0f;
		count = 1;
		v1 = v3;
		return;
	}

	if (d23_1 > 0.0f && d23_2 > 0.0f && d123_1 <= 0.0f)
	{
		F32 inv_d23 = 1.0f / (d23_1 + d23_2);
		v2.a = d23_1 * inv_d23;
		v3.a = d23_2 * inv_d23;
		count = 2;
		v1 = v3;
		return;
	}

	F32 inv_d123 = 1.0f / (d123_1 + d123_2 + d123_3);
	v1.a = d123_1 * inv_d123;
	v2.a = d123_2 * inv_d123;
	v3.a = d123_3 * inv_d123;
	count = 3;
}

F32 SeparationFunction::Initialize(const SimplexCache* cache,
	const DistanceProxy* proxyA_, const Sweep2D& sweepA_,
	const DistanceProxy* proxyB_, const Sweep2D& sweepB_,
	F32 t1)
{
	proxyA = proxyA_;
	proxyB = proxyB_;
	U32 count = cache->count;

	sweepA = sweepA_;
	sweepB = sweepB_;

	Transform2D xfA = sweepA.GetTransform(t1);
	Transform2D xfB = sweepB.GetTransform(t1);

	if (count == 1)
	{
		type = Points;
		Vector2 localPointA = proxyA->vertices[cache->indexA[0]];
		Vector2 localPointB = proxyB->vertices[cache->indexB[0]];
		Vector2 pointA = localPointA * xfA;
		Vector2 pointB = localPointB * xfB;
		axis = pointB - pointA;
		F32 s;
		axis.Normalize(s);
		return s;
	}
	else if (cache->indexA[0] == cache->indexA[1])
	{
		// Two points on B and one on A.
		type = FaceB;
		Vector2 localPointB1 = proxyB->vertices[cache->indexA[0]];
		Vector2 localPointB2 = proxyB->vertices[cache->indexB[1]];

		axis = (localPointB2 - localPointB1).Cross(1.0f);
		axis.Normalize();
		Vector2 normal = axis * xfB.rotation;

		localPoint = 0.5f * (localPointB1 + localPointB2);
		Vector2 pointB = localPoint * xfB;

		Vector2 localPointA = proxyA->vertices[cache->indexA[0]];
		Vector2 pointA = localPointA * xfA;

		F32 s = (pointA - pointB).Dot(normal);
		if (s < 0.0f)
		{
			axis = -axis;
			s = -s;
		}
		return s;
	}
	else
	{
		// Two points on A and one or two points on B.
		type = FaceA;
		Vector2 localPointA1 = proxyA->vertices[cache->indexA[0]];
		Vector2 localPointA2 = proxyA->vertices[cache->indexA[1]];

		axis = (localPointA2 - localPointA1).Cross(1.0f);
		axis.Normalize();
		Vector2 normal = axis * xfA.rotation;

		localPoint = 0.5f * (localPointA1 + localPointA2);
		Vector2 pointA = localPoint * xfA;

		Vector2 localPointB = proxyB->vertices[cache->indexB[0]];
		Vector2 pointB = localPointB * xfB;

		F32 s = (pointB - pointA).Dot(normal);
		if (s < 0.0f)
		{
			axis = -axis;
			s = -s;
		}
		return s;
	}
}

F32 SeparationFunction::FindMinSeparation(U32* indexA, U32* indexB, F32 t) const
{
	Transform2D xfA = sweepA.GetTransform(t);
	Transform2D xfB = sweepB.GetTransform(t);

	switch (type)
	{
	case Points:
	{
		Vector2 axisA = axis ^ xfA.rotation;
		Vector2 axisB = -axis ^ xfB.rotation;

		*indexA = proxyA->GetSupport(axisA);
		*indexB = proxyB->GetSupport(axisB);

		Vector2 localPointA = proxyA->vertices[*indexA];
		Vector2 localPointB = proxyB->vertices[*indexB];

		Vector2 pointA = localPointA * xfA;
		Vector2 pointB = localPointB * xfB;

		F32 separation = (pointB - pointA).Dot(axis);
		return separation;
	}

	case FaceA:
	{
		Vector2 normal = axis * xfA.rotation;
		Vector2 pointA = localPoint * xfA;

		Vector2 axisB = -normal ^ xfB.rotation;

		*indexA = -1;
		*indexB = proxyB->GetSupport(axisB);

		Vector2 localPointB = proxyB->vertices[*indexB];
		Vector2 pointB = localPointB * xfB;

		F32 separation = (pointB - pointA).Dot(normal);
		return separation;
	}

	case FaceB:
	{
		Vector2 normal = axis * xfB.rotation;
		Vector2 pointB = localPoint * xfB;

		Vector2 axisA = -normal ^ xfA.rotation;

		*indexB = -1;
		*indexA = proxyA->GetSupport(axisA);

		Vector2 localPointA = proxyA->vertices[*indexA];
		Vector2 pointA = localPointA * xfA;
		
		F32 separation = (pointA - pointB).Dot(normal);
		return separation;
	}

	default:
		*indexA = -1;
		*indexB = -1;
		return 0.0f;
	}
}

F32 SeparationFunction::Evaluate(U32 indexA, U32 indexB, F32 t) const
{
	Transform2D xfA = sweepA.GetTransform(t);
	Transform2D xfB = sweepB.GetTransform(t);

	switch (type)
	{
	case Points:
	{
		Vector2 localPointA = proxyA->vertices[indexA];
		Vector2 localPointB = proxyB->vertices[indexB];

		Vector2 pointA = localPointA * xfA;
		Vector2 pointB = localPointB * xfB;
		F32 separation = (pointB - pointA).Dot(axis);

		return separation;
	}

	case FaceA:
	{
		Vector2 normal = axis * xfA.rotation;
		Vector2 pointA = localPoint * xfA;

		Vector2 localPointB = proxyB->vertices[indexB];
		Vector2 pointB = localPointB * xfB;

		F32 separation = (pointB - pointA).Dot(normal);
		return separation;
	}

	case FaceB:
	{
		Vector2 normal = axis * xfB.rotation;
		Vector2 pointB = localPoint * xfB;

		Vector2 localPointA = proxyA->vertices[indexA];
		Vector2 pointA = localPointA * xfA;

		F32 separation = (pointA - pointB).Dot(normal);
		return separation;
	}

	default: return 0.0f;
	}
}

F32 FindMaxSeparation(U32* edgeIndex,
	const Collider2D::Polygon* poly1, const Transform2D& xf1,
	const Collider2D::Polygon* poly2, const Transform2D& xf2)
{
	U32 count1 = poly1->vertexCount;
	U32 count2 = poly2->vertexCount;
	const Vector2* n1s = poly1->normals;
	const Vector2* v1s = poly1->vertices;
	const Vector2* v2s = poly2->vertices;
	Transform2D xf = xf2 ^ xf1;

	U32 bestIndex = 0;
	F32 maxSeparation = -F32_MAX;
	for (U32 i = 0; i < count1; ++i)
	{
		// Get poly1 normal in frame2.
		Vector2 n = n1s[i] * xf.rotation;
		Vector2 v1 = v1s[i] * xf;

		// Find deepest point for normal i.
		F32 si = F32_MAX;
		for (U32 j = 0; j < count2; ++j)
		{
			F32 sij = n.Dot(v2s[j] - v1);
			if (sij < si)
			{
				si = sij;
			}
		}

		if (si > maxSeparation)
		{
			maxSeparation = si;
			bestIndex = i;
		}
	}

	*edgeIndex = bestIndex;
	return maxSeparation;
}

void FindIncidentEdge(ClipVertex c[2],
	const Collider2D::Polygon* poly1, const Transform2D& xf1, U32 edge1,
	const Collider2D::Polygon* poly2, const Transform2D& xf2)
{
	const Vector2* normals1 = poly1->normals;

	U32 count2 = poly2->vertexCount;
	const Vector2* vertices2 = poly2->vertices;
	const Vector2* normals2 = poly2->normals;

	// Get the normal of the reference edge in poly2's frame.
	Vector2 normal1 = (normals1[edge1] * xf1.rotation) ^ xf2.rotation;

	// Find the incident edge on poly2.
	U32 index = 0;
	F32 minDot = F32_MAX;
	for (U32 i = 0; i < count2; ++i)
	{
		F32 dot = normal1.Dot(normals2[i]);
		if (dot < minDot)
		{
			minDot = dot;
			index = i;
		}
	}

	// Build the clip vertices for the incident edge.
	U32 i1 = index;
	U32 i2 = i1 + 1 < count2 ? i1 + 1 : 0;

	c[0].v = vertices2[i1] * xf2;
	c[0].id.cf.indexA = (U8)edge1;
	c[0].id.cf.indexB = (U8)i1;
	c[0].id.cf.typeA = ContactFeature::Face;
	c[0].id.cf.typeB = ContactFeature::Vertex;

	c[1].v = vertices2[i2] * xf2;
	c[1].id.cf.indexA = (U8)edge1;
	c[1].id.cf.indexB = (U8)i2;
	c[1].id.cf.typeA = ContactFeature::Face;
	c[1].id.cf.typeB = ContactFeature::Vertex;
}

U32 ClipSegmentToLine(ClipVertex vOut[2], const ClipVertex vIn[2],
	const Vector2& normal, F32 offset, U32 vertexIndexA)
{
	// Start with no output points
	U32 numOut = 0;

	// Calculate the distance of end points to the line
	F32 distance0 = normal.Dot(vIn[0].v) - offset;
	F32 distance1 = normal.Dot(vIn[1].v) - offset;

	// If the points are behind the plane
	if (distance0 <= 0.0f) vOut[numOut++] = vIn[0];
	if (distance1 <= 0.0f) vOut[numOut++] = vIn[1];

	// If the points are on different sides of the plane
	if (distance0 * distance1 < 0.0f)
	{
		// Find intersection point of edge and plane
		F32 interp = distance0 / (distance0 - distance1);
		vOut[numOut].v = vIn[0].v + interp * (vIn[1].v - vIn[0].v);

		// VertexA is hitting edgeB.
		vOut[numOut].id.cf.indexA = (U8)vertexIndexA;
		vOut[numOut].id.cf.indexB = vIn[0].id.cf.indexB;
		vOut[numOut].id.cf.typeA = ContactFeature::Vertex;
		vOut[numOut].id.cf.typeB = ContactFeature::Face;
		++numOut;
	}

	return numOut;
}

void Evaluate(Manifold2D* manifold, const Collider2D* colliderA, const Transform2D& xfA, const Collider2D* colliderB, const Transform2D& xfB)
{
	switch (colliderA->type)
	{
	case COLLIDER_TYPE_CIRCLE: {
		switch (colliderB->type)
		{
		case COLLIDER_TYPE_CIRCLE: {
			manifold->pointCount = 0;

			Vector2 pA = colliderA->center * xfA;
			Vector2 pB = colliderB->center * xfB;

			Vector2 d = pB - pA;
			F32 distSqr = d.SqrMagnitude();
			F32 rA = colliderA->radius, rB = colliderB->radius;
			F32 radius = rA + rB;
			if (distSqr > radius * radius) { return; }

			manifold->type = Manifold2D::Circles;
			manifold->localPoint = colliderA->center;
			manifold->localNormal = Vector2Zero;
			manifold->pointCount = 1;

			manifold->points[0].localPoint = colliderB->center;
			manifold->points[0].id.key = 0;
		} break;
		case COLLIDER_TYPE_POLYGON: { BreakPoint; } break;
		}
	} break;
	case COLLIDER_TYPE_POLYGON: {
		switch (colliderB->type)
		{
		case COLLIDER_TYPE_CIRCLE: {
			const Collider2D::Polygon* polygonA = &colliderA->polygon;

			manifold->pointCount = 0;

			// Compute circle position in the frame of the polygon.
			Vector2 c = colliderB->center * xfB;
			Vector2 cLocal = c ^ xfA;

			// Find the min separating edge.
			U32 normalIndex = 0;
			F32 separation = -F32_MAX;
			F32 radius = colliderA->radius + colliderB->radius;
			U32 vertexCount = polygonA->vertexCount;
			const Vector2* vertices = polygonA->vertices;
			const Vector2* normals = polygonA->normals;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				F32 s = normals[i].Dot(cLocal - vertices[i]);

				if (s > radius) { return; }

				if (s > separation)
				{
					separation = s;
					normalIndex = i;
				}
			}

			// Vertices that subtend the incident face.
			U32 vertIndex1 = normalIndex;
			U32 vertIndex2 = vertIndex1 + 1 < vertexCount ? vertIndex1 + 1 : 0;
			Vector2 v1 = vertices[vertIndex1];
			Vector2 v2 = vertices[vertIndex2];

			// If the center is inside the polygon ...
			if (separation < Traits<F32>::Epsilon)
			{
				manifold->pointCount = 1;
				manifold->type = Manifold2D::FaceA;
				manifold->localNormal = normals[normalIndex];
				manifold->localPoint = 0.5f * (v1 + v2);
				manifold->points[0].localPoint = colliderB->center;
				manifold->points[0].id.key = 0;
				return;
			}

			// Compute barycentric coordinates
			F32 u1 = (cLocal - v1).Dot(v2 - v1);
			F32 u2 = (cLocal - v2).Dot(v1 - v2);
			if (u1 <= 0.0f)
			{
				if ((cLocal - v1).SqrMagnitude() > radius * radius) { return; }

				manifold->pointCount = 1;
				manifold->type = Manifold2D::FaceA;
				manifold->localNormal = cLocal - v1;
				manifold->localNormal.Normalize();
				manifold->localPoint = v1;
				manifold->points[0].localPoint = colliderB->center;
				manifold->points[0].id.key = 0;
			}
			else if (u2 <= 0.0f)
			{
				if ((cLocal - v2).SqrMagnitude() > radius * radius) { return; }

				manifold->pointCount = 1;
				manifold->type = Manifold2D::FaceA;
				manifold->localNormal = cLocal - v2;
				manifold->localNormal.Normalize();
				manifold->localPoint = v2;
				manifold->points[0].localPoint = colliderB->center;
				manifold->points[0].id.key = 0;
			}
			else
			{
				Vector2 faceCenter = 0.5f * (v1 + v2);
				F32 separation = (cLocal - faceCenter).Dot(normals[vertIndex1]);
				if (separation > radius) { return; }

				manifold->pointCount = 1;
				manifold->type = Manifold2D::FaceA;
				manifold->localNormal = normals[vertIndex1];
				manifold->localPoint = faceCenter;
				manifold->points[0].localPoint = colliderB->center;
				manifold->points[0].id.key = 0;
			}
		} break;
		case COLLIDER_TYPE_POLYGON: {
			const Collider2D::Polygon* polygonA = &colliderA->polygon;
			const Collider2D::Polygon* polygonB = &colliderB->polygon;

			manifold->pointCount = 0;
			F32 totalRadius = colliderA->radius + colliderB->radius;

			U32 edgeA = 0;
			F32 separationA = FindMaxSeparation(&edgeA, polygonA, xfA, polygonB, xfB);
			if (separationA > totalRadius) { return; }

			U32 edgeB = 0;
			F32 separationB = FindMaxSeparation(&edgeB, polygonB, xfB, polygonA, xfA);
			if (separationB > totalRadius) { return; }

			const Collider2D::Polygon* poly1;	// reference polygon
			const Collider2D::Polygon* poly2;	// incident polygon
			Transform2D xf1, xf2;
			U32 edge1;					// reference edge
			U8 flip;
			const F32 k_tol = 0.1f * LinearSlop;

			if (separationB > separationA + k_tol)
			{
				poly1 = polygonB;
				poly2 = polygonA;
				xf1 = xfB;
				xf2 = xfA;
				edge1 = edgeB;
				manifold->type = Manifold2D::FaceB;
				flip = 1;
			}
			else
			{
				poly1 = polygonA;
				poly2 = polygonB;
				xf1 = xfA;
				xf2 = xfB;
				edge1 = edgeA;
				manifold->type = Manifold2D::FaceA;
				flip = 0;
			}

			ClipVertex incidentEdge[2];
			FindIncidentEdge(incidentEdge, poly1, xf1, edge1, poly2, xf2);

			U32 count1 = poly1->vertexCount;
			const Vector2* vertices1 = poly1->vertices;

			U32 iv1 = edge1;
			U32 iv2 = edge1 + 1 < count1 ? edge1 + 1 : 0;

			Vector2 v11 = vertices1[iv1];
			Vector2 v12 = vertices1[iv2];

			Vector2 localTangent = v12 - v11;
			localTangent.Normalize();

			Vector2 localNormal = localTangent.Cross(1.0f);
			Vector2 planePoint = 0.5f * (v11 + v12);

			Vector2 tangent = localTangent * xf1.rotation;
			Vector2 normal = tangent.Cross(1.0f);

			v11 = v11 * xf1;
			v12 = v12 * xf1;

			// Face offset.
			F32 frontOffset = normal.Dot(v11);

			// Side offsets, extended by polytope skin thickness.
			F32 sideOffset1 = -tangent.Dot(v11) + totalRadius;
			F32 sideOffset2 = tangent.Dot(v12) + totalRadius;

			// Clip incident edge against extruded edge1 side edges.
			ClipVertex clipPoints1[2];
			ClipVertex clipPoints2[2];

			// Clip to box side 1
			if (ClipSegmentToLine(clipPoints1, incidentEdge, -tangent, sideOffset1, iv1) < 2) { return; }

			// Clip to negative box side 1
			if (ClipSegmentToLine(clipPoints2, clipPoints1, tangent, sideOffset2, iv2) < 2) { return; }

			// Now clipPoints2 contains the clipped points.
			manifold->localNormal = localNormal;
			manifold->localPoint = planePoint;

			U32 pointCount = 0;
			for (U32 i = 0; i < MaxManifoldPoints; ++i)
			{
				F32 separation = normal.Dot(clipPoints2[i].v) - frontOffset;

				if (separation <= totalRadius)
				{
					ManifoldPoint* cp = manifold->points + pointCount;
					cp->localPoint = clipPoints2[i].v ^ xf2;
					cp->id = clipPoints2[i].id;
					if (flip)
					{
						// Swap features
						ContactFeature cf = cp->id.cf;
						cp->id.cf.indexA = cf.indexB;
						cp->id.cf.indexB = cf.indexA;
						cp->id.cf.typeA = cf.typeB;
						cp->id.cf.typeB = cf.typeA;
					}
					++pointCount;
				}
			}

			manifold->pointCount = pointCount;
		} break;
		}
	} break;
	}
}

void Contact2D::Update()
{
	Manifold2D oldManifold = manifold;

	// Re-enable this contact.
	flags |= FLAG_ENABLED;

	bool touching = false;
	bool wasTouching = (flags & FLAG_TOUCHING);

	bool trigger = colliderA->trigger || colliderB->trigger;

	RigidBody2D* bodyA = colliderA->body;
	RigidBody2D* bodyB = colliderB->body;
	const Transform2D& xfA = bodyA->Transform();
	const Transform2D& xfB = bodyB->Transform();

	// Is this contact a trigger?
	if (trigger)
	{
		touching = Physics::TestOverlap(colliderA, colliderB, xfA, xfB);

		// triggers don't generate manifolds.
		manifold.pointCount = 0;
	}
	else
	{
		Evaluate(&manifold, colliderA, xfA, colliderB, xfB);
		touching = manifold.pointCount > 0;

		// Match old contact ids to new contact ids and copy the
		// stored impulses to warm start the solver.
		for (U32 i = 0; i < manifold.pointCount; ++i)
		{
			ManifoldPoint* mp2 = manifold.points + i;
			mp2->normalImpulse = 0.0f;
			mp2->tangentImpulse = 0.0f;
			ContactID id2 = mp2->id;

			for (U32 j = 0; j < oldManifold.pointCount; ++j)
			{
				ManifoldPoint* mp1 = oldManifold.points + j;

				if (mp1->id.key == id2.key)
				{
					mp2->normalImpulse = mp1->normalImpulse;
					mp2->tangentImpulse = mp1->tangentImpulse;
					break;
				}
			}
		}

		if (touching != wasTouching)
		{
			bodyA->SetAwake(true);
			bodyB->SetAwake(true);
		}
	}

	if (touching) { flags |= FLAG_TOUCHING; }
	else { flags &= ~FLAG_TOUCHING; }

	//TODO: Events
	//if (wasTouching == false && touching == true && listener)
	//{
	//	listener->BeginContact(this);
	//}
	//
	//if (wasTouching == true && touching == false && listener)
	//{
	//	listener->EndContact(this);
	//}
	//
	//if (sensor == false && touching && listener)
	//{
	//	listener->PreSolve(this, &oldManifold);
	//}
}