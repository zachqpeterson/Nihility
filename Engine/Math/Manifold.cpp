#include "Manifold.hpp"

#include "Physics.hpp"

#define B2_MAKE_ID(A, B) ((U8)(A) << 8 | (U8)(B))

Manifold CollideCircles(const Circle& circleA, const Transform2D& xfA, const Circle& circleB, const Transform2D& xfB)
{
	Manifold manifold = {};

	Transform2D xf = xfA ^ xfB;

	Vector2 pointA = circleA.center;
	Vector2 pointB = circleB.center * xf;

	F32 distance = (pointB - pointA).Magnitude();
	Vector2 normal = (pointB - pointA) / distance;

	F32 radiusA = circleA.radius;
	F32 radiusB = circleB.radius;

	F32 separation = distance - radiusA - radiusB;
	if (separation > SpeculativeDistance) { return manifold; }

	Vector2 cA = pointA + normal * radiusA;
	Vector2 cB = pointB + normal * -radiusB;
	Vector2 contactPointA = Math::Lerp(cA, cB, 0.5f);

	manifold.normal = normal.Rotate(xfA.rotation);
	ManifoldPoint* mp = manifold.points + 0;
	mp->anchorA = contactPointA.Rotate(xfA.rotation);
	mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
	mp->point = xfA.position + mp->anchorA;
	mp->separation = separation;
	mp->id = 0;
	manifold.pointCount = 1;
	return manifold;
}

Manifold CollideCapsuleAndCircle(const Capsule& capsuleA, const Transform2D& xfA, const Circle& circleB, const Transform2D& xfB)
{
	Manifold manifold = {};

	Transform2D xf = xfA ^ xfB;

	Vector2 pB = circleB.center * xf;

	Vector2 p1 = capsuleA.center1;
	Vector2 p2 = capsuleA.center2;

	Vector2 e = p2 - p1;

	Vector2 pA;
	F32 s1 = (pB - p1).Dot(e);
	F32 s2 = (p2 - pB).Dot(e);

	if (s1 < 0.0f) { pA = p1; }
	else if (s2 < 0.0f) { pA = p2; }
	else { pA = p1 + e * (s1 / e.Dot(e)); }

	F32 distance = (pB - pA).Magnitude();
	Vector2 normal = (pB - pA) / distance;

	F32 radiusA = capsuleA.radius;
	F32 radiusB = circleB.radius;
	F32 separation = distance - radiusA - radiusB;
	if (separation > SpeculativeDistance) { return manifold; }

	Vector2 cA = pA + normal * radiusA;
	Vector2 cB = pB + normal * -radiusB;
	Vector2 contactPointA = Math::Lerp(cA, cB, 0.5f);

	manifold.normal = normal * xfA.rotation;
	ManifoldPoint* mp = manifold.points + 0;
	mp->anchorA = contactPointA * xfA.rotation;
	mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
	mp->point = xfA.position + mp->anchorA;
	mp->separation = separation;
	mp->id = 0;
	manifold.pointCount = 1;
	return manifold;
}

Manifold CollideCapsules(const Capsule& capsuleA, const Transform2D& xfA, const Capsule& capsuleB, const Transform2D& xfB)
{
	Vector2 origin = capsuleA.center1;

	Transform2D sfA = { xfA.position + origin * xfA.rotation, xfA.rotation };
	Transform2D xf = sfA ^ xfB;

	Vector2 p1 = Vector2Zero;
	Vector2 q1 = capsuleA.center2 - origin;

	Vector2 p2 = capsuleB.center1 * xf;
	Vector2 q2 = capsuleB.center2 * xf;

	Vector2 d1 = q1 - p1;
	Vector2 d2 = q2 - p2;

	F32 dd1 = d1.Dot(d1);
	F32 dd2 = d2.Dot(d2);

	const F32 epsSqr = Traits<F32>::Epsilon * Traits<F32>::Epsilon;

	Vector2 r = p1 - p2;
	F32 rd1 = r.Dot(d1);
	F32 rd2 = r.Dot(d2);

	F32 d12 = d1.Dot(d2);

	F32 denom = dd1 * dd2 - d12 * d12;

	F32 f1 = 0.0f;
	if (denom != 0.0f)
	{
		f1 = Math::Clamp((d12 * rd2 - rd1 * dd2) / denom, 0.0f, 1.0f);
	}

	F32 f2 = (d12 * f1 + rd2) / dd2;

	if (f2 < 0.0f)
	{
		f2 = 0.0f;
		f1 = Math::Clamp(-rd1 / dd1, 0.0f, 1.0f);
	}
	else if (f2 > 1.0f)
	{
		f2 = 1.0f;
		f1 = Math::Clamp((d12 - rd1) / dd1, 0.0f, 1.0f);
	}

	Vector2 closest1 = p1 + d1 * f1;
	Vector2 closest2 = p2 + d2 * f2;
	F32 distanceSquared = (closest1 - closest2).SqrMagnitude();

	Manifold manifold = { 0 };
	F32 radiusA = capsuleA.radius;
	F32 radiusB = capsuleB.radius;
	F32 radius = radiusA + radiusB;
	F32 maxDistance = radius + SpeculativeDistance;

	if (distanceSquared > maxDistance * maxDistance) { return manifold; }

	F32 distance = Math::Sqrt(distanceSquared);

	F32 length1 = d1.Magnitude();
	F32 length2 = d2.Magnitude();
	Vector2 u1 = d1 / length1;
	Vector2 u2 = d2 / length2;

	F32 fp2 = (p2 - p1).Dot(u1);
	F32 fq2 = (q2 - p1).Dot(u1);
	bool outsideA = (fp2 <= 0.0f && fq2 <= 0.0f) || (fp2 >= length1 && fq2 >= length1);

	F32 fp1 = (p1 - p2).Dot(u2);
	F32 fq1 = (q1 - p2).Dot(u2);
	bool outsideB = (fp1 <= 0.0f && fq1 <= 0.0f) || (fp1 >= length2 && fq1 >= length2);

	if (outsideA == false && outsideB == false)
	{
		// attempt to clip
		// this may yield contact points with excessive separation
		// in that case the algorithm falls back to single point collision

		// find reference edge using SAT
		Vector2 normalA;
		F32 separationA;

		{
			normalA = u1.PerpendicularLeft();
			F32 ss1 = (p2 - p1).Dot(normalA);
			F32 ss2 = (q2 - p1).Dot(normalA);
			F32 s1p = ss1 < ss2 ? ss1 : ss2;
			F32 s1n = -ss1 < -ss2 ? -ss1 : -ss2;

			if (s1p > s1n)
			{
				separationA = s1p;
			}
			else
			{
				separationA = s1n;
				normalA = -normalA;
			}
		}

		Vector2 normalB;
		F32 separationB;
		{
			normalB = u2.PerpendicularLeft();
			F32 ss1 = (p1 - p2).Dot(normalB);
			F32 ss2 = (q1 - p2).Dot(normalB);
			F32 s1p = ss1 < ss2 ? ss1 : ss2;
			F32 s1n = -ss1 < -ss2 ? -ss1 : -ss2;

			if (s1p > s1n)
			{
				separationB = s1p;
			}
			else
			{
				separationB = s1n;
				normalB = -normalB;
			}
		}

		if (separationA >= separationB)
		{
			manifold.normal = normalA;

			Vector2 cp = p2;
			Vector2 cq = q2;

			if (fp2 < 0.0f && fq2 > 0.0f)
			{
				cp = Math::Lerp(p2, q2, (0.0f - fp2) / (fq2 - fp2));
			}
			else if (fq2 < 0.0f && fp2 > 0.0f)
			{
				cq = Math::Lerp(q2, p2, (0.0f - fq2) / (fp2 - fq2));
			}

			if (fp2 > length1 && fq2 < length1)
			{
				cp = Math::Lerp(p2, q2, (fp2 - length1) / (fp2 - fq2));
			}
			else if (fq2 > length1 && fp2 < length1)
			{
				cq = Math::Lerp(q2, p2, (fq2 - length1) / (fq2 - fp2));
			}

			F32 sp = (cp, p1).Dot(normalA);
			F32 sq = (cq, p1).Dot(normalA);

			if (sp <= distance + LinearSlop || sq <= distance + LinearSlop)
			{
				ManifoldPoint* mp;
				mp = manifold.points + 0;
				mp->anchorA = cp + normalA * (0.5f * (radiusA - radiusB - sp));
				mp->separation = sp - radius;
				mp->id = B2_MAKE_ID(0, 0);

				mp = manifold.points + 1;
				mp->anchorA = cq + normalA * (0.5f * (radiusA - radiusB - sq));
				mp->separation = sq - radius;
				mp->id = B2_MAKE_ID(0, 1);
				manifold.pointCount = 2;
			}
		}
		else
		{
			manifold.normal = -normalB;

			Vector2 cp = p1;
			Vector2 cq = q1;

			if (fp1 < 0.0f && fq1 > 0.0f)
			{
				cp = Math::Lerp(p1, q1, (0.0f - fp1) / (fq1 - fp1));
			}
			else if (fq1 < 0.0f && fp1 > 0.0f)
			{
				cq = Math::Lerp(q1, p1, (0.0f - fq1) / (fp1 - fq1));
			}

			if (fp1 > length2 && fq1 < length2)
			{
				cp = Math::Lerp(p1, q1, (fp1 - length2) / (fp1 - fq1));
			}
			else if (fq1 > length2 && fp1 < length2)
			{
				cq = Math::Lerp(q1, p1, (fq1 - length2) / (fq1 - fp1));
			}

			F32 sp = (cp - p2).Dot(normalB);
			F32 sq = (cq - p2).Dot(normalB);

			if (sp <= distance + LinearSlop || sq <= distance + LinearSlop)
			{
				ManifoldPoint* mp;
				mp = manifold.points + 0;
				mp->anchorA = cp + normalB * (0.5f * (radiusB - radiusA - sp));
				mp->separation = sp - radius;
				mp->id = B2_MAKE_ID(0, 0);

				mp = manifold.points + 1;
				mp->anchorA = cq + normalB * (0.5f * (radiusB - radiusA - sq));
				mp->separation = sq - radius;
				mp->id = B2_MAKE_ID(1, 0);
				manifold.pointCount = 2;
			}
		}
	}

	if (manifold.pointCount == 0)
	{
		Vector2 normal = closest2 - closest1;
		if (normal.Dot(normal) > epsSqr) { normal.Normalize(); }
		else { normal = u1.PerpendicularLeft(); }

		Vector2 c1 = closest1 + normal * radiusA;
		Vector2 c2 = closest2 + normal * -radiusB;

		I32 i1 = f1 == 0.0f ? 0 : 1;
		I32 i2 = f2 == 0.0f ? 0 : 1;

		manifold.normal = normal;
		manifold.points[0].anchorA = Math::Lerp(c1, c2, 0.5f);
		manifold.points[0].separation = Math::Sqrt(distanceSquared) - radius;
		manifold.points[0].id = B2_MAKE_ID(i1, i2);
		manifold.pointCount = 1;
	}

	if (manifold.pointCount > 0)
	{
		manifold.normal = manifold.normal * xfA.rotation;
		for (U32 i = 0; i < manifold.pointCount; ++i)
		{
			ManifoldPoint* mp = manifold.points + i;

			mp->anchorA = (mp->anchorA + origin) * xfA.rotation;
			mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
			mp->point = xfA.position + mp->anchorA;
		}
	}

	return manifold;
}

Manifold CollidePolygonAndCircle(const ConvexPolygon& polygonA, const Transform2D& xfA, const Circle& circleB, const Transform2D& xfB)
{
	Manifold manifold = {};
	const F32 speculativeDistance = SpeculativeDistance;

	Transform2D xf = xfA ^ xfB;

	// Compute circle position in the frame of the polygon.
	Vector2 c = circleB.center * xf;
	F32 radiusA = polygonA.radius;
	F32 radiusB = circleB.radius;
	F32 radius = radiusA + radiusB;

	// Find the min separating edge.
	I32 normalIndex = 0;
	F32 separation = -F32_MAX;
	I32 vertexCount = polygonA.count;
	const Vector2* vertices = polygonA.vertices;
	const Vector2* normals = polygonA.normals;

	for (I32 i = 0; i < vertexCount; ++i)
	{
		F32 s = normals[i].Dot(c - vertices[i]);
		if (s > separation)
		{
			separation = s;
			normalIndex = i;
		}
	}

	if (separation > radius + speculativeDistance) { return manifold; }

	// Vertices of the reference edge.
	I32 vertIndex1 = normalIndex;
	I32 vertIndex2 = vertIndex1 + 1 < vertexCount ? vertIndex1 + 1 : 0;
	Vector2 v1 = vertices[vertIndex1];
	Vector2 v2 = vertices[vertIndex2];

	// Compute barycentric coordinates
	F32 u1 = (c - v1).Dot(v2 - v1);
	F32 u2 = (c - v2).Dot(v1 - v2);

	if (u1 < 0.0f && separation > Traits<F32>::Epsilon)
	{
		// Circle center is closest to v1 and safely outside the polygon
		Vector2 normal = (c - v1).Normalized();
		separation = (c - v1).Dot(normal);
		if (separation > radius + speculativeDistance) { return manifold; }

		Vector2 cA = v1 + normal * radiusA;
		Vector2 cB = c - normal * radiusB;
		Vector2 contactPointA = Math::Lerp(cA, cB, 0.5f);

		manifold.normal = normal * xfA.rotation;
		ManifoldPoint* mp = manifold.points + 0;
		mp->anchorA = contactPointA * xfA.rotation;
		mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
		mp->point = xfA.position + mp->anchorA;
		mp->separation = (cB - cA).Dot(normal);
		mp->id = 0;
		manifold.pointCount = 1;
	}
	else if (u2 < 0.0f && separation > Traits<F32>::Epsilon)
	{
		// Circle center is closest to v2 and safely outside the polygon
		Vector2 normal = (c - v2).Normalized();
		separation = (c - v2).Dot(normal);
		if (separation > radius + speculativeDistance)
		{
			return manifold;
		}

		Vector2 cA = v2 + normal * radiusA;
		Vector2 cB = c - normal * radiusB;
		Vector2 contactPointA = Math::Lerp(cA, cB, 0.5f);

		manifold.normal = normal * xfA.rotation;
		ManifoldPoint* mp = manifold.points + 0;
		mp->anchorA = contactPointA * xfA.rotation;
		mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
		mp->point = xfA.position + mp->anchorA;
		mp->separation = (cB - cA).Dot(normal);
		mp->id = 0;
		manifold.pointCount = 1;
	}
	else
	{
		// Circle center is between v1 and v2. Center may be inside polygon
		Vector2 normal = normals[normalIndex];
		manifold.normal = normal * xfA.rotation;

		// cA is the projection of the circle center onto to the reference edge
		Vector2 cA = c + normal * (radiusA - (c - v1).Dot(normal));

		// cB is the deepest point on the circle with respect to the reference edge
		Vector2 cB = c - normal * radiusB;

		Vector2 contactPointA = Math::Lerp(cA, cB, 0.5f);

		// The contact point is the midpoint in world space
		ManifoldPoint* mp = manifold.points + 0;
		mp->anchorA = contactPointA * xfA.rotation;
		mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
		mp->point = xfA.position + mp->anchorA;
		mp->separation = separation - radius;
		mp->id = 0;
		manifold.pointCount = 1;
	}

	return manifold;
}

// Polygon clipper used to compute contact points when there are potentially two contact points.
Manifold ClipPolygons(const ConvexPolygon& polyA, const ConvexPolygon& polyB, I32 edgeA, I32 edgeB, bool flip)
{
	Manifold manifold = {};

	// reference polygon
	const ConvexPolygon* poly1;
	I32 i11, i12;

	// incident polygon
	const ConvexPolygon* poly2;
	I32 i21, i22;

	if (flip)
	{
		poly1 = &polyB;
		poly2 = &polyA;
		i11 = edgeB;
		i12 = edgeB + 1 < polyB.count ? edgeB + 1 : 0;
		i21 = edgeA;
		i22 = edgeA + 1 < polyA.count ? edgeA + 1 : 0;
	}
	else
	{
		poly1 = &polyA;
		poly2 = &polyB;
		i11 = edgeA;
		i12 = edgeA + 1 < polyA.count ? edgeA + 1 : 0;
		i21 = edgeB;
		i22 = edgeB + 1 < polyB.count ? edgeB + 1 : 0;
	}

	Vector2 normal = poly1->normals[i11];

	// Reference edge vertices
	Vector2 v11 = poly1->vertices[i11];
	Vector2 v12 = poly1->vertices[i12];

	// Incident edge vertices
	Vector2 v21 = poly2->vertices[i21];
	Vector2 v22 = poly2->vertices[i22];

	Vector2 tangent = normal.CrossInv(1.0f);

	F32 lower1 = 0.0f;
	F32 upper1 = (v12 - v11).Dot(tangent);

	// Incident edge points opposite of tangent due to CCW winding
	F32 upper2 = (v21 - v11).Dot(tangent);
	F32 lower2 = (v22 - v11).Dot(tangent);

	// This check can fail slightly due to mismatch with GJK code.
	// Perhaps fall back to a single point here? Otherwise we get two coincident points.
	// if (upper2 < lower1 || upper1 < lower2)
	//{
	//	// numeric failure
	//	B2_ASSERT(false);
	//	return manifold;
	//}

	Vector2 vLower;
	if (lower2 < lower1 && upper2 - lower2 > Traits<F32>::Epsilon)
	{
		vLower = Math::Lerp(v22, v21, (lower1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vLower = v22;
	}

	Vector2 vUpper;
	if (upper2 > upper1 && upper2 - lower2 > Traits<F32>::Epsilon)
	{
		vUpper = Math::Lerp(v22, v21, (upper1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vUpper = v21;
	}

	// todo vLower can be very close to vUpper, reduce to one point?

	F32 separationLower = (vLower - v11).Dot(normal);
	F32 separationUpper = (vUpper - v11).Dot(normal);

	F32 r1 = poly1->radius;
	F32 r2 = poly2->radius;

	// Put contact points at midpoint, accounting for radii
	vLower = vLower + normal * (0.5f * (r1 - r2 - separationLower));
	vUpper = vUpper + normal * (0.5f * (r1 - r2 - separationUpper));

	F32 radius = r1 + r2;

	if (flip == false)
	{
		manifold.normal = normal;
		ManifoldPoint* cp = manifold.points + 0;

		{
			cp->anchorA = vLower;
			cp->separation = separationLower - radius;
			cp->id = B2_MAKE_ID(i11, i22);
			manifold.pointCount += 1;
			cp += 1;
		}

		{
			cp->anchorA = vUpper;
			cp->separation = separationUpper - radius;
			cp->id = B2_MAKE_ID(i12, i21);
			manifold.pointCount += 1;
		}
	}
	else
	{
		manifold.normal = -normal;
		ManifoldPoint* cp = manifold.points + 0;

		{
			cp->anchorA = vUpper;
			cp->separation = separationUpper - radius;
			cp->id = B2_MAKE_ID(i21, i12);
			manifold.pointCount += 1;
			cp += 1;
		}

		{
			cp->anchorA = vLower;
			cp->separation = separationLower - radius;
			cp->id = B2_MAKE_ID(i22, i11);
			manifold.pointCount += 1;
		}
	}

	return manifold;
}

// Find the max separation between poly1 and poly2 using edge normals from poly1.
F32 FindMaxSeparation(I32* edgeIndex, const ConvexPolygon& poly1, const ConvexPolygon& poly2)
{
	I32 count1 = poly1.count;
	I32 count2 = poly2.count;
	const Vector2* n1s = poly1.normals;
	const Vector2* v1s = poly1.vertices;
	const Vector2* v2s = poly2.vertices;

	I32 bestIndex = 0;
	F32 maxSeparation = -F32_MAX;
	for (I32 i = 0; i < count1; ++i)
	{
		// Get poly1 normal in frame2.
		Vector2 n = n1s[i];
		Vector2 v1 = v1s[i];

		// Find the deepest point for normal i.
		F32 si = F32_MAX;
		for (I32 j = 0; j < count2; ++j)
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

SegmentDistanceResult SegmentDistance(const Vector2& p1, const Vector2& q1, const Vector2& p2, const Vector2& q2)
{
	SegmentDistanceResult result = { 0 };

	Vector2 d1 = q1 - p1;
	Vector2 d2 = q2 - p2;
	Vector2 r = p1 - p2;
	F32 dd1 = d1.Dot(d1);
	F32 dd2 = d2.Dot(d2);
	F32 rd1 = r.Dot(d1);
	F32 rd2 = r.Dot(d2);

	const F32 epsSqr = Traits<F32>::Epsilon * Traits<F32>::Epsilon;

	if (dd1 < epsSqr || dd2 < epsSqr)
	{
		// Handle all degeneracies
		if (dd1 >= epsSqr)
		{
			// Segment 2 is degenerate
			result.fraction1 = Math::Clamp(-rd1 / dd1, 0.0f, 1.0f);
			result.fraction2 = 0.0f;
		}
		else if (dd2 >= epsSqr)
		{
			// Segment 1 is degenerate
			result.fraction1 = 0.0f;
			result.fraction2 = Math::Clamp(rd2 / dd2, 0.0f, 1.0f);
		}
		else
		{
			result.fraction1 = 0.0f;
			result.fraction2 = 0.0f;
		}
	}
	else
	{
		// Non-degenerate segments
		F32 d12 = d1.Dot(d2);

		F32 denom = dd1 * dd2 - d12 * d12;

		// Fraction on segment 1
		F32 f1 = 0.0f;
		if (denom != 0.0f)
		{
			// not parallel
			f1 = Math::Clamp((d12 * rd2 - rd1 * dd2) / denom, 0.0f, 1.0f);
		}

		// Compute point on segment 2 closest to p1 + f1 * d1
		F32 f2 = (d12 * f1 + rd2) / dd2;

		// Clamping of segment 2 requires a do over on segment 1
		if (f2 < 0.0f)
		{
			f2 = 0.0f;
			f1 = Math::Clamp(-rd1 / dd1, 0.0f, 1.0f);
		}
		else if (f2 > 1.0f)
		{
			f2 = 1.0f;
			f1 = Math::Clamp((d12 - rd1) / dd1, 0.0f, 1.0f);
		}

		result.fraction1 = f1;
		result.fraction2 = f2;
	}

	result.closest1 = p1 + d1 * result.fraction1;
	result.closest2 = p2 + d2 * result.fraction2;
	result.distanceSquared = (result.closest1 - result.closest2).SqrMagnitude();
	return result;
}

Manifold CollidePolygons(const ConvexPolygon& polygonA, const Transform2D& xfA, const ConvexPolygon& polygonB, const Transform2D& xfB)
{
	Vector2 origin = polygonA.vertices[0];

	// Shift polyA to origin
	// pw = q * pb + p
	// pw = q * (pbs + origin) + p
	// pw = q * pbs + (p + q * origin)
	Transform2D sfA = { xfA.position + origin * xfA.rotation, xfA.rotation };
	Transform2D xf = sfA ^ xfB;

	ConvexPolygon localPolyA;
	localPolyA.count = polygonA.count;
	localPolyA.radius = polygonA.radius;
	localPolyA.vertices[0] = Vector2Zero;
	localPolyA.normals[0] = polygonA.normals[0];
	for (I32 i = 1; i < localPolyA.count; ++i)
	{
		localPolyA.vertices[i] = polygonA.vertices[i] - origin;
		localPolyA.normals[i] = polygonA.normals[i];
	}

	// Put polyB in polyA's frame to reduce round-off error
	ConvexPolygon localPolyB;
	localPolyB.count = polygonB.count;
	localPolyB.radius = polygonB.radius;
	for (I32 i = 0; i < localPolyB.count; ++i)
	{
		localPolyB.vertices[i] = polygonB.vertices[i] * xf;
		localPolyB.normals[i] = polygonB.normals[i] * xf.rotation;
	}

	I32 edgeA = 0;
	F32 separationA = FindMaxSeparation(&edgeA, localPolyA, localPolyB);

	I32 edgeB = 0;
	F32 separationB = FindMaxSeparation(&edgeB, localPolyB, localPolyA);

	F32 radius = localPolyA.radius + localPolyB.radius;

	if (separationA > SpeculativeDistance + radius || separationB > SpeculativeDistance + radius)
	{
		return {};
	}

	// Find incident edge
	bool flip;
	if (separationA >= separationB)
	{
		flip = false;

		Vector2 searchDirection = localPolyA.normals[edgeA];

		// Find the incident edge on polyB
		I32 count = localPolyB.count;
		const Vector2* normals = localPolyB.normals;
		edgeB = 0;
		F32 minDot = F32_MAX;
		for (I32 i = 0; i < count; ++i)
		{
			F32 dot = searchDirection.Dot(normals[i]);
			if (dot < minDot)
			{
				minDot = dot;
				edgeB = i;
			}
		}
	}
	else
	{
		flip = true;

		Vector2 searchDirection = localPolyB.normals[edgeB];

		// Find the incident edge on polyA
		I32 count = localPolyA.count;
		const Vector2* normals = localPolyA.normals;
		edgeA = 0;
		F32 minDot = F32_MAX;
		for (I32 i = 0; i < count; ++i)
		{
			F32 dot = searchDirection.Dot(normals[i]);
			if (dot < minDot)
			{
				minDot = dot;
				edgeA = i;
			}
		}
	}

	Manifold manifold = { 0 };

	// Using slop here to ensure vertex-vertex normal vectors can be safely normalized
	// todo this means edge clipping needs to handle slightly non-overlapping edges.
	if (separationA > 0.1f * LinearSlop || separationB > 0.1f * LinearSlop)
	{
		// Polygons are disjoint. Find closest points between reference edge and incident edge
		// Reference edge on polygon A
		I32 i11 = edgeA;
		I32 i12 = edgeA + 1 < localPolyA.count ? edgeA + 1 : 0;
		I32 i21 = edgeB;
		I32 i22 = edgeB + 1 < localPolyB.count ? edgeB + 1 : 0;

		Vector2 v11 = localPolyA.vertices[i11];
		Vector2 v12 = localPolyA.vertices[i12];
		Vector2 v21 = localPolyB.vertices[i21];
		Vector2 v22 = localPolyB.vertices[i22];

		SegmentDistanceResult result = SegmentDistance(v11, v12, v21, v22);

		if (result.fraction1 == 0.0f && result.fraction2 == 0.0f)
		{
			// v11 - v21
			Vector2 normal = v21 - v11;
			F32 distance = Math::Sqrt(result.distanceSquared);
			if (distance > SpeculativeDistance + radius) { return manifold; }
			F32 invDistance = 1.0f / distance;
			normal.x *= invDistance;
			normal.y *= invDistance;

			Vector2 c1 = v11 + normal * localPolyA.radius;
			Vector2 c2 = v21 + normal * -localPolyB.radius;

			manifold.normal = normal;
			manifold.points[0].anchorA = Math::Lerp(c1, c2, 0.5f);
			manifold.points[0].separation = distance - radius;
			manifold.points[0].id = B2_MAKE_ID(i11, i21);
			manifold.pointCount = 1;
		}
		else if (result.fraction1 == 0.0f && result.fraction2 == 1.0f)
		{
			// v11 - v22
			Vector2 normal = v22 - v11;
			F32 distance = Math::Sqrt(result.distanceSquared);
			if (distance > SpeculativeDistance + radius) { return manifold; }
			F32 invDistance = 1.0f / distance;
			normal.x *= invDistance;
			normal.y *= invDistance;

			Vector2 c1 = v11 + normal * localPolyA.radius;
			Vector2 c2 = v22 + normal * -localPolyB.radius;

			manifold.normal = normal;
			manifold.points[0].anchorA = Math::Lerp(c1, c2, 0.5f);
			manifold.points[0].separation = distance - radius;
			manifold.points[0].id = B2_MAKE_ID(i11, i22);
			manifold.pointCount = 1;
		}
		else if (result.fraction1 == 1.0f && result.fraction2 == 0.0f)
		{
			// v12 - v21
			Vector2 normal = v21 - v12;
			F32 distance = Math::Sqrt(result.distanceSquared);
			if (distance > SpeculativeDistance + radius) { return manifold; }
			F32 invDistance = 1.0f / distance;
			normal.x *= invDistance;
			normal.y *= invDistance;

			Vector2 c1 = v12 + normal * localPolyA.radius;
			Vector2 c2 = v21 + normal * -localPolyB.radius;

			manifold.normal = normal;
			manifold.points[0].anchorA = Math::Lerp(c1, c2, 0.5f);
			manifold.points[0].separation = distance - radius;
			manifold.points[0].id = B2_MAKE_ID(i12, i21);
			manifold.pointCount = 1;
		}
		else if (result.fraction1 == 1.0f && result.fraction2 == 1.0f)
		{
			// v12 - v22
			Vector2 normal = v22 - v12;
			F32 distance = Math::Sqrt(result.distanceSquared);
			if (distance > SpeculativeDistance + radius) { return manifold; }
			F32 invDistance = 1.0f / distance;
			normal.x *= invDistance;
			normal.y *= invDistance;

			Vector2 c1 = v12 + normal * localPolyA.radius;
			Vector2 c2 = v22 + normal * -localPolyB.radius;

			manifold.normal = normal;
			manifold.points[0].anchorA = Math::Lerp(c1, c2, 0.5f);
			manifold.points[0].separation = distance - radius;
			manifold.points[0].id = B2_MAKE_ID(i12, i22);
			manifold.pointCount = 1;
		}
		else
		{
			// Edge region
			manifold = ClipPolygons(localPolyA, localPolyB, edgeA, edgeB, flip);
		}
	}
	else
	{
		// Polygons overlap
		manifold = ClipPolygons(localPolyA, localPolyB, edgeA, edgeB, flip);
	}

	// Convert manifold to world space
	if (manifold.pointCount > 0)
	{
		manifold.normal = manifold.normal * xfA.rotation;
		for (U32 i = 0; i < manifold.pointCount; ++i)
		{
			ManifoldPoint* mp = manifold.points + i;

			// anchor points relative to shape origin in world space
			mp->anchorA = (mp->anchorA + origin) * xfA.rotation;
			mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
			mp->point = xfA.position + mp->anchorA;
		}
	}

	return manifold;
}

Manifold CollideChainSegmentAndCircle(const ChainSegment& segmentA, const Transform2D& xfA, const Circle& circleB, const Transform2D& xfB)
{
	Manifold manifold = { 0 };

	Transform2D xf = xfA ^ xfB;

	// Compute circle in frame of segment
	Vector2 pB = circleB.center * xf;

	Vector2 p1 = segmentA.segment.point1;
	Vector2 p2 = segmentA.segment.point2;
	Vector2 e = p2 - p1;

	// Normal points to the right
	F32 offset = e.PerpendicularRight().Dot(pB - p1);
	if (offset < 0.0f) { return manifold; }

	// Barycentric coordinates
	F32 u = e.Dot(p2 - pB);
	F32 v = e.Dot(pB - p1);

	Vector2 pA;

	if (v <= 0.0f)
	{
		// Behind point1?
		// Is pB in the Voronoi region of the previous edge?
		Vector2 prevEdge = p1 - segmentA.ghost1;
		F32 uPrev = prevEdge.Dot(pB - p1);
		if (uPrev <= 0.0f) { return manifold; }

		pA = p1;
	}
	else if (u <= 0.0f)
	{
		// Ahead of point2?
		Vector2 nextEdge = segmentA.ghost2 - p2;
		F32 vNext = nextEdge.Dot(pB - p2);

		// Is pB in the Voronoi region of the next edge?
		if (vNext > 0.0f) { return manifold; }

		pA = p2;
	}
	else
	{
		F32 ee = e.Dot(e);
		pA = { u * p1.x + v * p2.x, u * p1.y + v * p2.y };
		pA = ee > 0.0f ? (1.0f / ee) * pA : p1;
	}

	F32 distance = (pB - pA).Magnitude();
	Vector2 normal = (pB - pA) / distance;

	F32 radius = circleB.radius;
	F32 separation = distance - radius;
	if (separation > SpeculativeDistance) { return manifold; }

	Vector2 cA = pA;
	Vector2 cB = pB + normal * -radius;
	Vector2 contactPointA = Math::Lerp(cA, cB, 0.5f);

	manifold.normal = normal * xfA.rotation;

	ManifoldPoint* mp = manifold.points + 0;
	mp->anchorA = contactPointA * xfA.rotation;
	mp->anchorB = mp->anchorA + (xfA.position - xfB.position);
	mp->point = xfA.position + mp->anchorA;
	mp->separation = separation;
	mp->id = 0;
	manifold.pointCount = 1;
	return manifold;
}

Manifold ClipSegments(const Vector2& a1, const Vector2& a2, const Vector2& b1, const Vector2& b2, const Vector2& normal, F32 ra, F32 rb, U16 id1, U16 id2)
{
	Manifold manifold = { 0 };

	Vector2 tangent = normal.PerpendicularLeft();

	// Barycentric coordinates of each point relative to a1 along tangent
	F32 lower1 = 0.0f;
	F32 upper1 = (a2 - a1).Dot(tangent);

	// Incident edge points opposite of tangent due to CCW winding
	F32 upper2 = (b1 - a1).Dot(tangent);
	F32 lower2 = (b2 - a1).Dot(tangent);

	// Do segments overlap?
	if (upper2 < lower1 || upper1 < lower2) { return manifold; }

	Vector2 vLower;
	if (lower2 < lower1 && upper2 - lower2 > Traits<F32>::Epsilon)
	{
		vLower = Math::Lerp(b2, b1, (lower1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vLower = b2;
	}

	Vector2 vUpper;
	if (upper2 > upper1 && upper2 - lower2 > Traits<F32>::Epsilon)
	{
		vUpper = Math::Lerp(b2, b1, (upper1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vUpper = b1;
	}

	// todo vLower can be very close to vUpper, reduce to one point?

	F32 separationLower = (vLower - a1).Dot(normal);
	F32 separationUpper = (vUpper - a1).Dot(normal);

	// Put contact points at midpoint, accounting for radii
	vLower = vLower + normal * (0.5f * (ra - rb - separationLower));
	vUpper = vUpper + normal * (0.5f * (ra - rb - separationUpper));

	F32 radius = ra + rb;

	manifold.normal = normal;
	{
		ManifoldPoint* cp = manifold.points + 0;
		cp->anchorA = vLower;
		cp->separation = separationLower - radius;
		cp->id = id1;
	}

	{
		ManifoldPoint* cp = manifold.points + 1;
		cp->anchorA = vUpper;
		cp->separation = separationUpper - radius;
		cp->id = id2;
	}

	manifold.pointCount = 2;

	return manifold;
}

enum NormalType
{
	// This means the normal points in a direction that is non-smooth relative to a convex vertex and should be skipped
	NORMAL_TYPE_SKIP,

	// This means the normal points in a direction that is smooth relative to a convex vertex and should be used for collision
	NORMAL_TYPE_ADMIT,

	// This means the normal is in a region of a concave vertex and should be snapped to the segment normal
	NORMAL_TYPE_SNAP
};

struct ChainSegmentParams
{
	Vector2 edge1;
	Vector2 normal0;
	Vector2 normal2;
	bool convex1;
	bool convex2;
};

// Evaluate Gauss map
// See https://box2d.org/posts/2020/06/ghost-collisions/
enum NormalType ClassifyNormal(ChainSegmentParams params, const Vector2& normal)
{
	const F32 sinTol = 0.01f;

	if (normal.Dot(params.edge1) <= 0.0f)
	{
		// Normal points towards the segment tail
		if (params.convex1)
		{
			if (normal.Cross(params.normal0) > sinTol) { return NORMAL_TYPE_SKIP; }

			return NORMAL_TYPE_ADMIT;
		}
		else { return NORMAL_TYPE_SNAP; }
	}
	else
	{
		// Normal points towards segment head
		if (params.convex2)
		{
			if (params.normal2.Cross(normal) > sinTol) { return NORMAL_TYPE_SKIP; }

			return NORMAL_TYPE_ADMIT;
		}
		else { return NORMAL_TYPE_SNAP; }
	}
}

DistanceProxy MakeProxy(const Vector2* vertices, I32 count, F32 radius)
{
	count = Math::Min((U8)count, MaxPolygonVertices);
	DistanceProxy proxy;
	for ( int i = 0; i < count; ++i )
	{
		proxy.points[i] = vertices[i];
	}
	proxy.count = count;
	proxy.radius = radius;
	return proxy;
}

Manifold CollideChainSegmentAndPolygon(const ChainSegment& segmentA, const Transform2D& xfA, const ConvexPolygon& polygonB, const Transform2D& xfB, DistanceCache& cache)
{
	Manifold manifold = { 0 };

	Transform2D xf = xfA ^ xfB;

	Vector2 centroidB = polygonB.centroid * xf;
	F32 radiusB = polygonB.radius;

	Vector2 p1 = segmentA.segment.point1;
	Vector2 p2 = segmentA.segment.point2;

	Vector2 edge1 = (p2 - p1).Normalized();

	ChainSegmentParams smoothParams = {};
	smoothParams.edge1 = edge1;

	const F32 convexTol = 0.01f;
	Vector2 edge0 = (p1 - segmentA.ghost1).Normalized();
	smoothParams.normal0 = edge0.PerpendicularRight();
	smoothParams.convex1 = edge0.Cross(edge1) >= convexTol;

	Vector2 edge2 = (segmentA.ghost2 - p2).Normalized();
	smoothParams.normal2 = edge2.PerpendicularRight();
	smoothParams.convex2 = edge1.Cross(edge2) >= convexTol;

	// Normal points to the right
	Vector2 normal1 = edge1.PerpendicularRight();
	bool behind1 = normal1.Dot(centroidB - p1) < 0.0f;
	bool behind0 = true;
	bool behind2 = true;
	if (smoothParams.convex1)
	{
		behind0 = smoothParams.normal0.Dot(centroidB - p1) < 0.0f;
	}

	if (smoothParams.convex2)
	{
		behind2 = smoothParams.normal2.Dot(centroidB - p2) < 0.0f;
	}

	if (behind1 && behind0 && behind2) { return manifold; }

	// Get polygonB in frameA
	I32 count = polygonB.count;
	Vector2 vertices[MaxPolygonVertices];
	Vector2 normals[MaxPolygonVertices];
	for (I32 i = 0; i < count; ++i)
	{
		vertices[i] = polygonB.vertices[i] * xf;
		normals[i] = polygonB.normals[i] * xf.rotation;
	}

	// Distance doesn't work correctly with partial polygons
	DistanceInput input;
	input.proxyA = MakeProxy(&segmentA.segment.point1, 2, 0.0f);
	input.proxyB = MakeProxy(vertices, count, 0.0f);
	input.transformA = Transform2DIdentity;
	input.transformB = Transform2DIdentity;
	input.useRadii = false;

	DistanceOutput output = Physics::ShapeDistance(cache, input, nullptr, 0);

	if (output.distance > radiusB + SpeculativeDistance) { return manifold; }

	// Snap concave normals for partial polygon
	Vector2 n0 = smoothParams.convex1 ? smoothParams.normal0 : normal1;
	Vector2 n2 = smoothParams.convex2 ? smoothParams.normal2 : normal1;

	// Index of incident vertex on polygon
	I32 incidentIndex = -1;
	I32 incidentNormal = -1;

	if (behind1 == false && output.distance > 0.1f * LinearSlop)
	{
		// The closest features may be two vertices or an edge and a vertex even when there should
		// be face contact

		if (cache.count == 1)
		{
			// vertex-vertex collision
			Vector2 pA = output.pointA;
			Vector2 pB = output.pointB;

			Vector2 normal = (pB - pA).Normalized();

			NormalType type = ClassifyNormal(smoothParams, normal);
			if (type == NORMAL_TYPE_SKIP)
			{
				return manifold;
			}

			if (type == NORMAL_TYPE_ADMIT)
			{
				manifold.normal = normal * xfA.rotation;
				ManifoldPoint* cp = manifold.points + 0;
				cp->anchorA = pA * xfA.rotation;
				cp->anchorB = cp->anchorA + (xfA.position - xfB.position);
				cp->point = xfA.position + cp->anchorA;
				cp->separation = output.distance - radiusB;
				cp->id = B2_MAKE_ID(cache.indexA[0], cache.indexB[0]);
				manifold.pointCount = 1;
				return manifold;
			}

			// fall through NORMAL_TYPE_SNAP
			incidentIndex = cache.indexB[0];
		}
		else
		{
			I32 ia1 = cache.indexA[0];
			I32 ia2 = cache.indexA[1];
			I32 ib1 = cache.indexB[0];
			I32 ib2 = cache.indexB[1];

			if (ia1 == ia2)
			{
				// Find polygon normal most aligned with vector between closest points.
				// This effectively sorts ib1 and ib2
				Vector2 normalB = output.pointA - output.pointB;
				F32 dot1 = normalB.Dot(normals[ib1]);
				F32 dot2 = normalB.Dot(normals[ib2]);
				I32 ib = dot1 > dot2 ? ib1 : ib2;

				// Use accurate normal
				normalB = normals[ib];

				NormalType type = ClassifyNormal(smoothParams, -normalB);
				if (type == NORMAL_TYPE_SKIP)
				{
					return manifold;
				}

				if (type == NORMAL_TYPE_ADMIT)
				{
					// Get polygon edge associated with normal
					ib1 = ib;
					ib2 = ib < count - 1 ? ib + 1 : 0;

					Vector2 b1 = vertices[ib1];
					Vector2 b2 = vertices[ib2];

					// Find incident segment vertex
					dot1 = normalB.Dot(p1 - b1);
					dot2 = normalB.Dot(p2 - b1);

					if (dot1 < dot2)
					{
						if (n0.Dot(normalB) < normal1.Dot(normalB))
						{
							// Neighbor is incident
							return manifold;
						}
					}
					else
					{
						if (n2.Dot(normalB) < normal1.Dot(normalB))
						{
							// Neighbor is incident
							return manifold;
						}
					}

					manifold = ClipSegments(b1, b2, p1, p2, normalB, radiusB, 0.0f, B2_MAKE_ID(ib1, 1), B2_MAKE_ID(ib2, 0));
					manifold.normal = -normalB * xfA.rotation;
					manifold.points[0].anchorA = manifold.points[0].anchorA * xfA.rotation;
					manifold.points[1].anchorA = manifold.points[1].anchorA * xfA.rotation;
					Vector2 pAB = xfA.position - xfB.position;
					manifold.points[0].anchorB = manifold.points[0].anchorA + pAB;
					manifold.points[1].anchorB = manifold.points[1].anchorA + pAB;
					manifold.points[0].point = xfA.position + manifold.points[0].anchorA;
					manifold.points[1].point = xfA.position + manifold.points[1].anchorA;
					return manifold;
				}

				// fall through NORMAL_TYPE_SNAP
				incidentNormal = ib;
			}
			else
			{
				// Get index of incident polygonB vertex
				F32 dot1 = normal1.Dot(vertices[ib1] - p1);
				F32 dot2 = normal1.Dot(vertices[ib2] - p2);
				incidentIndex = dot1 < dot2 ? ib1 : ib2;
			}
		}
	}
	else
	{
		// SAT edge normal
		F32 edgeSeparation = F32_MAX;

		for (I32 i = 0; i < count; ++i)
		{
			F32 s = normal1.Dot(vertices[i] - p1);
			if (s < edgeSeparation)
			{
				edgeSeparation = s;
				incidentIndex = i;
			}
		}

		// Check convex neighbor for edge separation
		if (smoothParams.convex1)
		{
			F32 s0 = F32_MAX;

			for (I32 i = 0; i < count; ++i)
			{
				F32 s = smoothParams.normal0.Dot(vertices[i] - p1);
				if (s < s0)
				{
					s0 = s;
				}
			}

			if (s0 > edgeSeparation)
			{
				edgeSeparation = s0;

				// Indicate neighbor owns edge separation
				incidentIndex = -1;
			}
		}

		// Check convex neighbor for edge separation
		if (smoothParams.convex2)
		{
			F32 s2 = F32_MAX;

			for (I32 i = 0; i < count; ++i)
			{
				F32 s = smoothParams.normal2.Dot(vertices[i] - p2);
				if (s < s2)
				{
					s2 = s;
				}
			}

			if (s2 > edgeSeparation)
			{
				edgeSeparation = s2;

				// Indicate neighbor owns edge separation
				incidentIndex = -1;
			}
		}

		// SAT polygon normals
		F32 polygonSeparation = -F32_MAX;
		I32 referenceIndex = -1;

		for (I32 i = 0; i < count; ++i)
		{
			Vector2 n = normals[i];

			NormalType type = ClassifyNormal(smoothParams, -n);
			if (type != NORMAL_TYPE_ADMIT) { continue; }

			// Check the infinite sides of the partial polygon
			// if ((smoothParams.convex1 && b2Cross(n0, n) > 0.0f) || (smoothParams.convex2 && b2Cross(n, n2) > 0.0f))
			//{
			//	continue;
			//}

			Vector2 p = vertices[i];
			F32 s = Math::Min(n.Dot(p2 - p), n.Dot(p1 - p));

			if (s > polygonSeparation)
			{
				polygonSeparation = s;
				referenceIndex = i;
			}
		}

		if (polygonSeparation > edgeSeparation)
		{
			I32 ia1 = referenceIndex;
			I32 ia2 = ia1 < count - 1 ? ia1 + 1 : 0;
			Vector2 a1 = vertices[ia1];
			Vector2 a2 = vertices[ia2];

			Vector2 n = normals[ia1];

			F32 dot1 = n.Dot(p1 - a1);
			F32 dot2 = n.Dot(p2 - a1);

			if (dot1 < dot2)
			{
				if (n0.Dot(n) < normal1.Dot(n)) { return manifold; }
			}
			else
			{
				if (n2.Dot(n) < normal1.Dot(n)) { return manifold; }
			}

			manifold = ClipSegments(a1, a2, p1, p2, normals[ia1], radiusB, 0.0f, B2_MAKE_ID(ia1, 1), B2_MAKE_ID(ia2, 0));
			manifold.normal = -normals[ia1] * xfA.rotation;
			manifold.points[0].anchorA = manifold.points[0].anchorA * xfA.rotation;
			manifold.points[1].anchorA = manifold.points[1].anchorA * xfA.rotation;
			Vector2 pAB = xfA.position - xfB.position;
			manifold.points[0].anchorB = manifold.points[0].anchorA + pAB;
			manifold.points[1].anchorB = manifold.points[1].anchorA + pAB;
			manifold.points[0].point = xfA.position + manifold.points[0].anchorA;
			manifold.points[1].point = xfA.position + manifold.points[1].anchorA;
			return manifold;
		}

		if (incidentIndex == -1) { return manifold; }

		// fall through segment normal axis
	}

	// Segment normal

	// Find incident polygon normal: normal adjacent to deepest vertex that is most anti-parallel to segment normal
	Vector2 b1, b2;
	I32 ib1, ib2;

	if (incidentNormal != -1)
	{
		ib1 = incidentNormal;
		ib2 = ib1 < count - 1 ? ib1 + 1 : 0;
		b1 = vertices[ib1];
		b2 = vertices[ib2];
	}
	else
	{
		I32 i2 = incidentIndex;
		I32 i1 = i2 > 0 ? i2 - 1 : count - 1;
		F32 d1 = normal1.Dot(normals[i1]);
		F32 d2 = normal1.Dot(normals[i2]);
		if (d1 < d2)
		{
			ib1 = i1, ib2 = i2;
			b1 = vertices[ib1];
			b2 = vertices[ib2];
		}
		else
		{
			ib1 = i2, ib2 = i2 < count - 1 ? i2 + 1 : 0;
			b1 = vertices[ib1];
			b2 = vertices[ib2];
		}
	}

	manifold = ClipSegments(p1, p2, b1, b2, normal1, 0.0f, radiusB, B2_MAKE_ID(0, ib2), B2_MAKE_ID(1, ib1));
	manifold.normal = manifold.normal * xfA.rotation;
	manifold.points[0].anchorA = manifold.points[0].anchorA * xfA.rotation;
	manifold.points[1].anchorA = manifold.points[1].anchorA * xfA.rotation;
	Vector2 pAB = xfA.position - xfB.position;
	manifold.points[0].anchorB = manifold.points[0].anchorA + pAB;
	manifold.points[1].anchorB = manifold.points[1].anchorA + pAB;
	manifold.points[0].point = xfA.position + manifold.points[0].anchorA;
	manifold.points[1].point = xfA.position + manifold.points[1].anchorA;

	return manifold;
}

ConvexPolygon MakeCapsule(const Vector2& p1, const Vector2& p2, float radius)
{
	ConvexPolygon shape = { 0 };
	shape.vertices[0] = p1;
	shape.vertices[1] = p2;
	shape.centroid = Math::Lerp(p1, p2, 0.5f);

	Vector2 d = p2 - p1;
	Vector2 axis = d.Normalized();
	Vector2 normal = axis.PerpendicularRight();

	shape.normals[0] = normal;
	shape.normals[1] = -normal;
	shape.count = 2;
	shape.radius = radius;

	return shape;
}

Manifold CollideChainSegmentAndCapsule(const ChainSegment& segmentA, const Transform2D& xfA, const Capsule& capsuleB, const Transform2D& xfB, DistanceCache& cache)
{
	ConvexPolygon polyB = MakeCapsule(capsuleB.center1, capsuleB.center2, capsuleB.radius);
	return CollideChainSegmentAndPolygon(segmentA, xfA, polyB, xfB, cache);
}

Manifold CollideSegmentAndCapsule(const Segment& segmentA, const Transform2D& xfA, const Capsule& capsuleB, const Transform2D& xfB)
{
	Capsule capsuleA = { segmentA.point1, segmentA.point2, 0.0f };
	return CollideCapsules(capsuleA, xfA, capsuleB, xfB);
}

Manifold CollidePolygonAndCapsule(const ConvexPolygon& polygonA, const Transform2D& xfA, const Capsule& capsuleB, const Transform2D& xfB)
{
	ConvexPolygon polyB = MakeCapsule(capsuleB.center1, capsuleB.center2, capsuleB.radius);
	return CollidePolygons(polygonA, xfA, polyB, xfB);
}

Manifold CollideSegmentAndCircle(const Segment& segmentA, const Transform2D& xfA, const Circle& circleB, const Transform2D& xfB)
{
	Capsule capsuleA = { segmentA.point1, segmentA.point2, 0.0f };
	return CollideCapsuleAndCircle(capsuleA, xfA, circleB, xfB);
}

Manifold CollideSegmentAndPolygon(const Segment& segmentA, const Transform2D& xfA, const ConvexPolygon& polygonB, const Transform2D& xfB)
{
	ConvexPolygon polygonA = MakeCapsule(segmentA.point1, segmentA.point2, 0.0f);
	return CollidePolygons(polygonA, xfA, polygonB, xfB);
}

Manifold Manifold::CircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideCircles(shapeA.circle, xfA, shapeB.circle, xfB);
}

Manifold Manifold::CapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideCapsules(shapeA.capsule, xfA, shapeB.capsule, xfB);
}

Manifold Manifold::CapsuleAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideCapsuleAndCircle(shapeA.capsule, xfA, shapeB.circle, xfB);
}

Manifold Manifold::PolygonManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollidePolygons(shapeA.polygon, xfA, shapeB.polygon, xfB);
}

Manifold Manifold::PolygonAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollidePolygonAndCircle(shapeA.polygon, xfA, shapeB.circle, xfB);
}

Manifold Manifold::PolygonAndCapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollidePolygonAndCapsule(shapeA.polygon, xfA, shapeB.capsule, xfB);
}

Manifold Manifold::SegmentAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideSegmentAndCircle(shapeA.segment, xfA, shapeB.circle, xfB);
}

Manifold Manifold::SegmentAndCapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideSegmentAndCapsule(shapeA.segment, xfA, shapeB.capsule, xfB);
}

Manifold Manifold::SegmentAndPolygonManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideSegmentAndPolygon(shapeA.segment, xfA, shapeB.polygon, xfB);
}

Manifold Manifold::ChainSegmentAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideChainSegmentAndCircle(shapeA.chainSegment, xfA, shapeB.circle, xfB);
}

Manifold Manifold::ChainSegmentAndCapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideChainSegmentAndCapsule(shapeA.chainSegment, xfA, shapeB.capsule, xfB, cache);
}

Manifold Manifold::ChainSegmentAndPolygonManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	return CollideChainSegmentAndPolygon(shapeA.chainSegment, xfA, shapeB.polygon, xfB, cache);
}
