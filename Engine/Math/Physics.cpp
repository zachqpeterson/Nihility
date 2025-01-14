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

#include "Physics.hpp"

#include "Island.hpp"
#include "Broadphase.hpp"

#include "Core\Logger.hpp"
#include "Resources\Scene.hpp"
#include "Containers\Vector.hpp"
#include "Containers\Freelist.hpp"

#if NH_SIMD_WIDTH == 8
#define SIMD_SHIFT 3
#else
#define SIMD_SHIFT 2
#endif

ConstraintGraph Physics::constraintGraph;
Freelist Physics::rigidBodyFreelist(256);
Vector<RigidBody2D> Physics::rigidBodies(16);
Freelist Physics::solverSetFreelist(8);
Vector<SolverSet> Physics::solverSets(8);
Freelist Physics::jointFreelist(256);
Vector<Joint> Physics::joints(16);
Freelist Physics::contactFreelist(256);
Vector<Contact> Physics::contacts(16);
Freelist Physics::islandFreelist(256);
Vector<Island> Physics::islands(8);
Freelist Physics::shapeFreelist(256);
Vector<Shape> Physics::shapes(16);
Freelist Physics::chainFreelist(256);
Vector<ChainShape> Physics::chains(4);
Vector<TaskContext> Physics::taskContexts(1); //worker count
Vector<BodyMoveEvent> Physics::bodyMoveEvents(4);
Vector<SensorBeginTouchEvent> Physics::sensorBeginEvents(4);
Vector<SensorEndTouchEvent> Physics::sensorEndEvents(4);
Vector<ContactBeginTouchEvent> Physics::contactBeginEvents(4);
Vector<ContactEndTouchEvent> Physics::contactEndEvents(4);
Vector<ContactHitEvent> Physics::contactHitEvents(4);
U64 Physics::stepIndex = 0;
I32 Physics::splitIslandId = NullIndex;
Vector2 Physics::gravity = Vector2{ 0.0f, -9.8f };
F32 Physics::hitEventThreshold = 1.0f;
F32 Physics::restitutionThreshold = 1.0f;
F32 Physics::maxLinearVelocity = 400.0f;
F32 Physics::contactPushoutVelocity = 3.0f;
F32 Physics::contactHertz = 30.0f;
F32 Physics::contactDampingRatio = 10.0f;
F32 Physics::jointHertz = 60.0f;
F32 Physics::jointDampingRatio = 2.0f;
U16 Physics::revision;
I32 Physics::workerCount = 1;
void* Physics::userTreeTask = nullptr;
F32 Physics::inv_h;
I32 Physics::activeTaskCount = 0;
I32 Physics::taskCount = 0;
bool Physics::enableSleep = true;
bool Physics::locked = false;
bool Physics::enableWarmStarting = true;
bool Physics::enableContinuous = true;
bool Physics::paused = false;
bool Physics::singleStep = false;
int Physics::subStepCount = 4;

bool Physics::Initialize()
{
	Logger::Trace("Initializing Physics...");

	Broadphase::Initialize();
	constraintGraph.Create(16);

	SolverSet set{};
	set.setIndex = solverSetFreelist.GetFree();
	solverSets.Push(set);

	set.setIndex = solverSetFreelist.GetFree();
	solverSets.Push(set);

	set.setIndex = solverSetFreelist.GetFree();
	solverSets.Push(set);

	for (int i = 0; i < workerCount; ++i)
	{
		taskContexts[i].contactStateBitset.Create(1024);
		taskContexts[i].enlargedSimBitset.Create(256);
		taskContexts[i].awakeIslandBitset.Create(256);
	}

	return true;
}

void Physics::Shutdown()
{
	Logger::Trace("Shutting Down Physics...");

	for (int i = 0; i < workerCount; ++i)
	{
		taskContexts[i].contactStateBitset.Destroy();
		taskContexts[i].enlargedSimBitset.Destroy();
		taskContexts[i].awakeIslandBitset.Destroy();
	}

	taskContexts.Destroy();
	bodyMoveEvents.Destroy();
	sensorBeginEvents.Destroy();
	sensorEndEvents.Destroy();
	contactBeginEvents.Destroy();
	contactEndEvents.Destroy();
	contactHitEvents.Destroy();

	for (ChainShape& chain : chains)
	{
		Memory::Free(&chain.shapeIndices);
	}

	shapes.Destroy();
	chains.Destroy();
	contacts.Destroy();
	joints.Destroy();
	islands.Destroy();

	for (SolverSet& set : solverSets)
	{
		solverSetFreelist.Release(set.setIndex);
		set.Destroy();
	}

	solverSets.Destroy();

	constraintGraph.Destroy();

	Broadphase::Shutdown();

	solverSetFreelist.Destroy();
	jointFreelist.Destroy();
	contactFreelist.Destroy();
	islandFreelist.Destroy();
	shapeFreelist.Destroy();
	chainFreelist.Destroy();
}

void Physics::Update(F32 step)
{
	if (paused)
	{
		if (singleStep) { singleStep = false; }
		else { step = 0.0f; }
	}

	EnableSleeping(enableSleep);
	EnableWarmStarting(enableWarmStarting);
	EnableContinuous(enableContinuous);

	Step(step, subStepCount);
}

ConvexPolygon Physics::CreatePolygon(const Hull& hull, F32 radius)
{
	if (hull.count < 3)
	{
		// Handle a bad hull when assertions are disabled
		return CreateBox(0.5f, 0.5f);
	}

	ConvexPolygon shape;
	shape.count = hull.count;
	shape.radius = radius;

	// Copy vertices
	for (I32 i = 0; i < shape.count; ++i)
	{
		shape.vertices[i] = hull.points[i];
	}

	// Compute normals. Ensure the edges have non-zero length.
	for (I32 i = 0; i < shape.count; ++i)
	{
		I32 i1 = i;
		I32 i2 = i + 1 < shape.count ? i + 1 : 0;
		Vector2 edge = shape.vertices[i2] - shape.vertices[i1];
		shape.normals[i] = edge.Cross(1.0f).Normalized();
	}

	shape.centroid = ComputePolygonCentroid(shape.vertices, shape.count);

	return shape;
}

ConvexPolygon Physics::CreateOffsetPolygon(const Hull& hull, F32 radius, const Transform2D& transform)
{
	if (hull.count < 3)
	{
		// Handle a bad hull when assertions are disabled
		return CreateBox(0.5f, 0.5f);
	}

	ConvexPolygon shape;
	shape.count = hull.count;
	shape.radius = radius;

	// Copy vertices
	for (I32 i = 0; i < shape.count; ++i)
	{
		shape.vertices[i] = hull.points[i] * transform;
	}

	// Compute normals. Ensure the edges have non-zero length.
	for (I32 i = 0; i < shape.count; ++i)
	{
		I32 i1 = i;
		I32 i2 = i + 1 < shape.count ? i + 1 : 0;
		Vector2 edge = shape.vertices[i2] - shape.vertices[i1];
		shape.normals[i] = edge.Cross(1.0f).Normalized();
	}

	shape.centroid = ComputePolygonCentroid(shape.vertices, shape.count);

	return shape;
}

ConvexPolygon Physics::CreateBox(F32 hx, F32 hy)
{
	ConvexPolygon shape;
	shape.count = 4;
	shape.vertices[0] = { -hx, -hy };
	shape.vertices[1] = { hx, -hy };
	shape.vertices[2] = { hx, hy };
	shape.vertices[3] = { -hx, hy };
	shape.normals[0] = { 0.0f, -1.0f };
	shape.normals[1] = { 1.0f, 0.0f };
	shape.normals[2] = { 0.0f, 1.0f };
	shape.normals[3] = { -1.0f, 0.0f };
	shape.radius = 0.0f;
	shape.centroid = Vector2Zero;
	return shape;
}

ConvexPolygon Physics::CreateRoundedBox(F32 hx, F32 hy, F32 radius)
{
	ConvexPolygon shape = CreateBox(hx, hy);
	shape.radius = radius;
	return shape;
}

ConvexPolygon Physics::CreateOffsetBox(F32 hx, F32 hy, const Transform2D& transform)
{
	ConvexPolygon shape;
	shape.count = 4;
	shape.vertices[0] = Vector2{ -hx, -hy } * transform;
	shape.vertices[1] = Vector2{ hx, -hy } * transform;
	shape.vertices[2] = Vector2{ hx, hy } * transform;
	shape.vertices[3] = Vector2{ -hx, hy } * transform;
	shape.normals[0] = Vector2{ 0.0f, -1.0f } * transform.rotation;
	shape.normals[1] = Vector2{ 1.0f, 0.0f } * transform.rotation;
	shape.normals[2] = Vector2{ 0.0f, 1.0f } * transform.rotation;
	shape.normals[3] = Vector2{ -1.0f, 0.0f } * transform.rotation;
	shape.radius = 0.0f;
	shape.centroid = transform.position;
	return shape;
}

Vector2 Physics::ComputePolygonCentroid(const Vector2* vertices, I32 count)
{
	Vector2 center = { 0.0f, 0.0f };
	F32 area = 0.0f;

	// Get a reference point for forming triangles.
	// Use the first vertex to reduce round-off errors.
	Vector2 origin = vertices[0];

	const F32 inv3 = 1.0f / 3.0f;

	for (I32 i = 1; i < count - 1; ++i)
	{
		// Triangle edges
		Vector2 e1 = vertices[i] - origin;
		Vector2 e2 = vertices[i + 1] - origin;
		F32 a = 0.5f * e1.Cross(e2);

		// Area weighted centroid
		center = center + (e1 + e2) * a * inv3;
		area += a;
	}

	F32 invArea = 1.0f / area;
	center.x *= invArea;
	center.y *= invArea;

	// Restore offset
	center = origin + center;

	return center;
}

MassData Physics::ComputeCircleMass(const Circle& shape, F32 density)
{
	F32 rr = shape.radius * shape.radius;

	MassData massData;
	massData.mass = density * PI_F * rr;
	massData.center = shape.center;

	// inertia about the local origin
	massData.rotationalInertia = massData.mass * (0.5f * rr + shape.center.Dot(shape.center));

	return massData;
}

MassData Physics::ComputeCapsuleMass(const Capsule& shape, F32 density)
{
	float radius = shape.radius;
	float rr = radius * radius;
	Vector2 p1 = shape.center1;
	Vector2 p2 = shape.center2;
	float length = (p2 - p1).Magnitude();
	float ll = length * length;

	float circleMass = density * (PI_F * radius * radius);
	float boxMass = density * (2.0f * radius * length);

	MassData massData;
	massData.mass = circleMass + boxMass;
	massData.center.x = 0.5f * (p1.x + p2.x);
	massData.center.y = 0.5f * (p1.y + p2.y);

	// two offset half circles, both halves add up to full circle and each half is offset by half length
	// semi-circle centroid = 4 r / 3 pi
	// Need to apply parallel-axis theorem twice:
	// 1. shift semi-circle centroid to origin
	// 2. shift semi-circle to box end
	// m * ((h + lc)^2 - lc^2) = m * (h^2 + 2 * h * lc)
	// See: https://en.wikipedia.org/wiki/Parallel_axis_theorem
	// I verified this formula by computing the convex hull of a 128 vertex capsule

	// half circle centroid
	float lc = 4.0f * radius / (3.0f * PI_F);

	// half length of rectangular portion of capsule
	float h = 0.5f * length;

	float circleInertia = circleMass * (0.5f * rr + h * h + 2.0f * h * lc);
	float boxInertia = boxMass * (4.0f * rr + ll) / 12.0f;
	massData.rotationalInertia = circleInertia + boxInertia;

	// inertia about the local origin
	massData.rotationalInertia += massData.mass * massData.center.Dot(massData.center);

	return massData;
}

MassData Physics::ComputePolygonMass(const ConvexPolygon& shape, F32 density)
{
	// Polygon mass, centroid, and inertia.
	// Let rho be the polygon density in mass per unit area.
	// Then:
	// mass = rho * int(dA)
	// centroid.x = (1/mass) * rho * int(x * dA)
	// centroid.y = (1/mass) * rho * int(y * dA)
	// I = rho * int((x*x + y*y) * dA)
	//
	// We can compute these integrals by summing all the integrals
	// for each triangle of the polygon. To evaluate the integral
	// for a single triangle, we make a change of variables to
	// the (u,v) coordinates of the triangle:
	// x = x0 + e1x * u + e2x * v
	// y = y0 + e1y * u + e2y * v
	// where 0 <= u && 0 <= v && u + v <= 1.
	//
	// We integrate u from [0,1-v] and then v from [0,1].
	// We also need to use the Jacobian of the transformation:
	// D = cross(e1, e2)
	//
	// Simplification: triangle centroid = (1/3) * (p1 + p2 + p3)
	//
	// The rest of the derivation is handled by computer algebra.

	if (shape.count == 1)
	{
		Circle circle;
		circle.center = shape.vertices[0];
		circle.radius = shape.radius;
		return ComputeCircleMass(circle, density);
	}

	if (shape.count == 2)
	{
		Capsule capsule;
		capsule.center1 = shape.vertices[0];
		capsule.center2 = shape.vertices[1];
		capsule.radius = shape.radius;
		return ComputeCapsuleMass(capsule, density);
	}

	Vector2 vertices[MaxPolygonVertices] = { 0 };
	I32 count = shape.count;
	float radius = shape.radius;

	if (radius > 0.0f)
	{
		// Approximate mass of rounded polygons by pushing out the vertices.
		float sqrt2 = 1.412f;
		for (I32 i = 0; i < count; ++i)
		{
			I32 j = i == 0 ? count - 1 : i - 1;
			Vector2 n1 = shape.normals[j];
			Vector2 n2 = shape.normals[i];

			Vector2 mid = (n1 + n2).Normalized();
			vertices[i] = shape.vertices[i] + mid * sqrt2 * radius;
		}
	}
	else
	{
		for (I32 i = 0; i < count; ++i)
		{
			vertices[i] = shape.vertices[i];
		}
	}

	Vector2 center = { 0.0f, 0.0f };
	float area = 0.0f;
	float rotationalInertia = 0.0f;

	// Get a reference point for forming triangles.
	// Use the first vertex to reduce round-off errors.
	Vector2 r = vertices[0];

	const float inv3 = 1.0f / 3.0f;

	for (I32 i = 1; i < count - 1; ++i)
	{
		// Triangle edges
		Vector2 e1 = vertices[i] - r;
		Vector2 e2 = vertices[i + 1] - r;

		float D = e1.Cross(e2);

		float triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid, r at origin
		center = center + (e1 + e2) * triangleArea * inv3;

		float ex1 = e1.x, ey1 = e1.y;
		float ex2 = e2.x, ey2 = e2.y;

		float intx2 = ex1 * ex1 + ex2 * ex1 + ex2 * ex2;
		float inty2 = ey1 * ey1 + ey2 * ey1 + ey2 * ey2;

		rotationalInertia += (0.25f * inv3 * D) * (intx2 + inty2);
	}

	MassData massData;

	// Total mass
	massData.mass = density * area;

	// Center of mass, shift back from origin at r
	float invArea = 1.0f / area;
	center.x *= invArea;
	center.y *= invArea;
	massData.center = r + center;

	// Inertia tensor relative to the local origin (point s).
	massData.rotationalInertia = density * rotationalInertia;

	// Shift to center of mass then to original body origin.
	massData.rotationalInertia += massData.mass * (massData.center.Dot(massData.center) - center.Dot(center));

	return massData;
}

Shape& Physics::CreateCircleShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const Circle& geometry)
{
	int shapeId = shapeFreelist.GetFree();

	if (shapeId == shapes.Size()) { shapes.Push({ def }); }

	Shape& shape = shapes[shapeId];

	shape.id = shapeId;
	shape.bodyId = body.id;
	shape.type = SHAPE_TYPE_CIRCLE;
	shape.circle = geometry;

	CreateShape(body, transform, def, shape);

	return shape;
}

Shape& Physics::CreateCapsuleShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const Capsule& geometry)
{
	int shapeId = shapeFreelist.GetFree();

	if (shapeId == shapes.Size()) { shapes.Push({ def }); }

	Shape& shape = shapes[shapeId];

	shape.id = shapeId;
	shape.bodyId = body.id;
	shape.type = SHAPE_TYPE_CAPSULE;
	shape.capsule = geometry;

	CreateShape(body, transform, def, shape);

	return shape;
}

Shape& Physics::CreateConvexPolygonShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const ConvexPolygon& geometry)
{
	int shapeId = shapeFreelist.GetFree();

	if (shapeId == shapes.Size()) { shapes.Push({ def }); }

	Shape& shape = shapes[shapeId];

	shape.id = shapeId;
	shape.bodyId = body.id;
	shape.type = SHAPE_TYPE_POLYGON;
	shape.polygon = geometry;

	CreateShape(body, transform, def, shape);

	return shape;
}

Shape& Physics::CreateSegmentShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const Segment& geometry)
{
	int shapeId = shapeFreelist.GetFree();

	if (shapeId == shapes.Size()) { shapes.Push({ def }); }

	Shape& shape = shapes[shapeId];

	shape.id = shapeId;
	shape.bodyId = body.id;
	shape.type = SHAPE_TYPE_SEGMENT;
	shape.segment = geometry;

	CreateShape(body, transform, def, shape);

	return shape;
}

Shape& Physics::CreateChainSegmentShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const ChainSegment& geometry)
{
	int shapeId = shapeFreelist.GetFree();

	if (shapeId == shapes.Size()) { shapes.Push({ def }); }

	Shape& shape = shapes[shapeId];

	shape.id = shapeId;
	shape.bodyId = body.id;
	shape.type = SHAPE_TYPE_CHAIN_SEGMENT;
	shape.chainSegment = geometry;

	CreateShape(body, transform, def, shape);

	return shape;
}

void Physics::CreateShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, Shape& shape)
{
	if (body.setIndex != SET_TYPE_DISABLED)
	{
		shape.UpdateShapeAABBs(transform, body.type);
		shape.proxyKey = Broadphase::CreateProxy(body.type, shape.fatAABB, shape.filter.layers, shape.id, def.forceContactCreation || def.isSensor);
	}

	// Add to shape doubly linked list
	if (body.headShapeId != NullIndex)
	{
		Shape& headShape = shapes[body.headShapeId];
		headShape.prevShapeId = shape.id;
	}

	shape.prevShapeId = NullIndex;
	shape.nextShapeId = body.headShapeId;
	body.headShapeId = shape.id;
	body.shapeCount += 1;
}

ShapeExtent Physics::ComputeShapeExtent(const Shape& shape, Vector2 localCenter)
{
	ShapeExtent extent;

	switch (shape.type)
	{
	case SHAPE_TYPE_CAPSULE: {
		F32 radius = shape.capsule.radius;
		extent.minExtent = radius;
		Vector2 c1 = shape.capsule.center1 - localCenter;
		Vector2 c2 = shape.capsule.center2 - localCenter;
		extent.maxExtent = Math::Sqrt(Math::Max(c1.SqrMagnitude(), c2.SqrMagnitude())) + radius;
	} break;
	case SHAPE_TYPE_CIRCLE: {
		F32 radius = shape.circle.radius;
		extent.minExtent = radius;
		extent.maxExtent = (shape.circle.center - localCenter).Magnitude() + radius;
	} break;
	case SHAPE_TYPE_POLYGON: {
		const ConvexPolygon& poly = shape.polygon;
		F32 minExtent = Huge;
		F32 maxExtentSqr = 0.0f;
		int count = poly.count;
		for (int i = 0; i < count; ++i)
		{
			Vector2 v = poly.vertices[i];
			F32 planeOffset = poly.normals[i].Dot(v - poly.centroid);
			minExtent = Math::Min(minExtent, planeOffset);

			F32 distanceSqr = (v - localCenter).SqrMagnitude();
			maxExtentSqr = Math::Max(maxExtentSqr, distanceSqr);
		}

		extent.minExtent = minExtent + poly.radius;
		extent.maxExtent = Math::Sqrt(maxExtentSqr) + poly.radius;
	} break;
	case SHAPE_TYPE_SEGMENT: {
		extent.minExtent = 0.0f;
		Vector2 c1 = shape.segment.point1 - localCenter;
		Vector2 c2 = shape.segment.point2 - localCenter;
		extent.maxExtent = Math::Sqrt(Math::Max(c1.SqrMagnitude(), c2.SqrMagnitude()));
	} break;
	case SHAPE_TYPE_CHAIN_SEGMENT: {
		extent.minExtent = 0.0f;
		Vector2 c1 = shape.chainSegment.segment.point1 - localCenter;
		Vector2 c2 = shape.chainSegment.segment.point2 - localCenter;
		extent.maxExtent = Math::Sqrt(Math::Max(c1.SqrMagnitude(), c2.SqrMagnitude()));
	} break;
	default: break;
	}

	return extent;
}

RigidBody2D& Physics::GetRigidBody(I32 id)
{
	return rigidBodies[id];
}

void Physics::EnableSleeping(bool flag)
{
	if (locked || flag == enableSleep) { return; }

	enableSleep = flag;

	if (flag == false)
	{
		I32 setCount = (I32)solverSets.Size();
		for (I32 i = SET_TYPE_FIRST_SLEEPING; i < setCount; ++i)
		{
			if (solverSets[i].bodySims.Size())
			{
				WakeSolverSet(i);
			}
		}
	}
}

void Physics::EnableWarmStarting(bool flag)
{
	if (locked) { return; }

	enableWarmStarting = flag;
}

void Physics::EnableContinuous(bool flag)
{
	if (locked) { return; }

	enableContinuous = flag;
}

void Physics::WakeSolverSet(int setIndex)
{
	SolverSet& set = solverSets[setIndex];
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];
	SolverSet& disabledSet = solverSets[SET_TYPE_DISABLED];

	for (BodySim& simSrc : set.bodySims)
	{
		RigidBody2D& body = rigidBodies[simSrc.bodyId];
		body.setIndex = SET_TYPE_AWAKE;
		body.localIndex = (I32)awakeSet.bodySims.Size();

		// Reset sleep timer
		body.sleepTime = 0.0f;

		awakeSet.bodySims.Push(simSrc);
		awakeSet.bodyStates.Push(BodyStateIdentity);

		// move non-touching contacts from disabled set to awake set
		int contactKey = body.headContactKey;
		while (contactKey != NullIndex)
		{
			int edgeIndex = contactKey & 1;
			int contactId = contactKey >> 1;

			Contact& contact = contacts[contactId];

			contactKey = contact.edges[edgeIndex].nextKey;

			if (contact.setIndex != SET_TYPE_DISABLED) { continue; }

			int localIndex = contact.localIndex;
			ContactSim& contactSim = disabledSet.contactSims[localIndex];

			contact.setIndex = SET_TYPE_AWAKE;
			contact.localIndex = (I32)awakeSet.contactSims.Size();
			awakeSet.contactSims.Push(contactSim);

			int movedLocalIndex = disabledSet.contactSims.RemoveSwap(localIndex);
			if (movedLocalIndex != -1)
			{
				// fix moved element
				ContactSim& movedContactSim = disabledSet.contactSims[localIndex];
				Contact& movedContact = contacts[movedContactSim.contactId];
				movedContact.localIndex = localIndex;
			}
		}
	}

	// transfer touching contacts from sleeping set to contact graph
	for (ContactSim& contactSim : set.contactSims)
	{
		Contact& contact = contacts[contactSim.contactId];
		constraintGraph.AddContact(contactSim, contact);
		contact.setIndex = SET_TYPE_AWAKE;
	}

	// transfer joints from sleeping set to awake set
	for (JointSim& jointSim : set.jointSims)
	{
		Joint& joint = joints[jointSim.jointId];
		constraintGraph.AddJoint(jointSim, joint);
		joint.setIndex = SET_TYPE_AWAKE;
	}

	// transfer island from sleeping set to awake set
	// Usually a sleeping set has only one island, but it is possible
	// that joints are created between sleeping islands and they
	// are moved to the same sleeping set.
	for (IslandSim& islandSrc : set.islandSims)
	{
		Island& island = islands[islandSrc.islandId];
		island.setIndex = SET_TYPE_AWAKE;
		island.localIndex = (I32)awakeSet.islandSims.Size();
		awakeSet.islandSims.Push(islandSrc);
	}

	set.Destroy();
}

void Physics::Step(F32 timeStep, int subStepCount)
{
	if (locked) { return; }

	// Prepare to capture events
	// Ensure user does not access stale data if there is an early return
	bodyMoveEvents.Clear();
	sensorBeginEvents.Clear();
	sensorEndEvents.Clear();
	contactBeginEvents.Clear();
	contactEndEvents.Clear();
	contactHitEvents.Clear();

	if (Math::IsZero(timeStep)) { return; }

	locked = true;
	activeTaskCount = 0;
	taskCount = 0;

	// Update collision pairs and create contacts
	Broadphase::Update();

	StepContext context{};
	context.dt = timeStep;
	context.subStepCount = Math::Max(1, subStepCount);

	if (timeStep > 0.0f)
	{
		context.inv_dt = 1.0f / timeStep;
		context.h = timeStep / context.subStepCount;
		context.inv_h = context.subStepCount * context.inv_dt;
	}
	else
	{
		context.inv_dt = 0.0f;
		context.h = 0.0f;
		context.inv_h = 0.0f;
	}

	inv_h = context.inv_h;

	// Hertz values get reduced for large time steps
	F32 contactHertz = Math::Min(contactHertz, 0.25f * context.inv_h);
	F32 jointHertz = Math::Min(jointHertz, 0.125f * context.inv_h);

	context.contactSoftness.Create(contactHertz, contactDampingRatio, context.h);
	context.staticSoftness.Create(2.0f * contactHertz, contactDampingRatio, context.h);
	context.jointSoftness.Create(jointHertz, jointDampingRatio, context.h);

	context.restitutionThreshold = restitutionThreshold;
	context.maxLinearVelocity = maxLinearVelocity;
	context.enableWarmStarting = enableWarmStarting;

	Collide(context);
	Solve(context);

	locked = false;
}

void Physics::AddNonTouchingContact(Contact& contact, ContactSim& contactSim)
{
	SolverSet& set = solverSets[SET_TYPE_AWAKE];
	contact.colorIndex = NullIndex;
	contact.localIndex = (I32)set.contactSims.Size();

	set.contactSims.Push(contactSim);
}

void Physics::RemoveNonTouchingContact(int setIndex, int localIndex)
{
	SolverSet& set = solverSets[setIndex];
	int movedIndex = set.contactSims.RemoveSwap(localIndex);
	if (movedIndex != NullIndex)
	{
		ContactSim& movedContactSim = set.contactSims[localIndex];
		Contact& movedContact = contacts[movedContactSim.contactId];
		movedContact.localIndex = localIndex;
	}
}

BodySim& Physics::GetBodySim(RigidBody2D& body)
{
	SolverSet& set = solverSets[body.setIndex];
	BodySim& bodySim = set.bodySims[body.localIndex];
	return bodySim;
}

// Narrow-phase collision
void Physics::Collide(StepContext& context)
{
	// Tasks that can be done in parallel with the narrow-phase
	// - rebuild the collision tree for dynamic and kinematic bodies to keep their query performance good
	Broadphase::RebuildTrees();
	taskCount += 1;

	// gather contacts into a single array for easier parallel-for
	I32 contactCount = 0;
	GraphColor* graphColors = constraintGraph.colors;
	for (I32 i = 0; i < GraphColorCount; ++i)
	{
		contactCount += (I32)graphColors[i].contactSims.Size();
	}

	I32 nonTouchingCount = (I32)solverSets[SET_TYPE_AWAKE].contactSims.Size();
	contactCount += nonTouchingCount;

	if (contactCount == 0) { return; }

	Vector<ContactSim*> contactSims(contactCount);

	I32 contactIndex = 0;
	for (I32 i = 0; i < GraphColorCount; ++i)
	{
		GraphColor* color = graphColors + i;
		I32 count = (I32)color->contactSims.Size();
		ContactSim* base = color->contactSims.Data();
		for (I32 j = 0; j < count; ++j)
		{
			contactSims[contactIndex] = base + j;
			contactIndex += 1;
		}
	}

	{
		ContactSim* base = solverSets[SET_TYPE_AWAKE].contactSims.Data();
		for (I32 i = 0; i < nonTouchingCount; ++i)
		{
			contactSims[contactIndex] = base + i;
			contactIndex += 1;
		}
	}

	context.contacts = Move(contactSims);

	// Contact bit set on ids because contact pointers are unstable as they move between touching and not touching.
	I32 contactIdCapacity = contactFreelist.Capacity();
	for (I32 i = 0; i < workerCount; ++i)
	{
		taskContexts[i].contactStateBitset.SetBitCountAndClear(contactIdCapacity);
	}

	// Task should take at least 40us on a 4GHz CPU (10K cycles)
	I32 minRange = 64;
	CollideTask(0, contactCount, 0, context);
	taskCount += 1;

	context.contacts.Destroy();

	// Bitwise OR all contact bits
	Bitset& bitset = taskContexts[0].contactStateBitset;
	for (I32 i = 1; i < workerCount; ++i)
	{
		bitset.InPlaceUnion(taskContexts[i].contactStateBitset);
	}

	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];

	const Shape* shapesArr = shapes.Data();

	// Process contact state changes. Iterate over set bits
	for (U32 k = 0; k < bitset.blockCount; ++k)
	{
		U64 bits = bitset.bits[k];
		while (bits != 0)
		{
			UL32 ctz;
			_BitScanForward64(&ctz, bits);
			int contactId = (int)(64 * k + ctz);

			Contact& contact = contacts[contactId];

			int colorIndex = contact.colorIndex;
			int localIndex = contact.localIndex;

			ContactSim* contactSim = nullptr;
			if (colorIndex != NullIndex)
			{
				// contact lives in constraint graph
				GraphColor& color = graphColors[colorIndex];
				contactSim = &color.contactSims[localIndex];
			}
			else
			{
				contactSim = &awakeSet.contactSims[localIndex];
			}

			const Shape* shapeA = shapesArr + contact.shapeIdA;
			const Shape* shapeB = shapesArr + contact.shapeIdB;
			I32 shapeIdA = shapeA->id + 1;
			I32 shapeIdB = shapeB->id + 1;
			U32 flags = contact.flags;
			U32 simFlags = contactSim->simFlags;

			if (simFlags & CONTACT_SIM_FLAG_DISJOINT)
			{
				// Was touching?
				if ((flags & CONTACT_FLAG_TOUCHING) != 0 && (flags & CONTACT_FLAG_ENABLE_CONTACT_EVENTS) != 0)
				{
					contactEndEvents.Push({ shapeIdA, shapeIdB });
				}

				// Bounding boxes no longer overlap
				contact.flags &= ~CONTACT_FLAG_TOUCHING;
				DestroyContact(contact, false);
			}
			else if (simFlags & CONTACT_SIM_FLAG_STARTED_TOUCHING)
			{
				if ((flags & CONTACT_FLAG_SENSOR) != 0)
				{
					// Contact is a sensor
					if ((flags & CONTACT_FLAG_ENABLE_SENSOR_EVENTS) != 0)
					{
						if (shapeA->isSensor) { sensorBeginEvents.Push({ shapeIdA, shapeIdB }); }

						if (shapeB->isSensor) { sensorBeginEvents.Push({ shapeIdB, shapeIdA }); }
					}

					contactSim->simFlags &= ~CONTACT_SIM_FLAG_STARTED_TOUCHING;
					contact.flags |= CONTACT_FLAG_SENSOR_TOUCHING;
				}
				else
				{
					// Contact is solid
					if (flags & CONTACT_FLAG_ENABLE_CONTACT_EVENTS)
					{
						contactBeginEvents.Push({ shapeIdA, shapeIdB, contactSim->manifold });
					}

					// Link first because this wakes colliding bodies and ensures the body sims
					// are in the correct place.
					contact.flags |= CONTACT_FLAG_TOUCHING;
					LinkContact(contact);

					// Contact sim pointer may have become orphaned due to awake set growth,
					// so I just need to refresh it.
					contactSim = &awakeSet.contactSims[localIndex];

					contactSim->simFlags &= ~CONTACT_SIM_FLAG_STARTED_TOUCHING;

					constraintGraph.AddContact(*contactSim, contact);
					RemoveNonTouchingContact(SET_TYPE_AWAKE, localIndex);
					contactSim = nullptr;
				}
			}
			else if (simFlags & CONTACT_SIM_FLAG_STOPPED_TOUCHING)
			{
				contactSim->simFlags &= ~CONTACT_SIM_FLAG_STOPPED_TOUCHING;

				if ((flags & CONTACT_FLAG_SENSOR) != 0)
				{
					// Contact is a sensor
					contact.flags &= ~CONTACT_FLAG_SENSOR_TOUCHING;

					if ((flags & CONTACT_FLAG_ENABLE_SENSOR_EVENTS) != 0)
					{
						if (shapeA->isSensor) { sensorEndEvents.Push({ shapeIdA, shapeIdB }); }

						if (shapeB->isSensor) { sensorEndEvents.Push({ shapeIdB, shapeIdA }); }
					}
				}
				else
				{
					// Contact is solid
					contact.flags &= ~CONTACT_FLAG_TOUCHING;

					if (contact.flags & CONTACT_FLAG_ENABLE_CONTACT_EVENTS)
					{
						contactEndEvents.Push({ shapeIdA, shapeIdB });
					}

					UnlinkContact(contact);
					int bodyIdA = contact.edges[0].bodyId;
					int bodyIdB = contact.edges[1].bodyId;

					AddNonTouchingContact(contact, *contactSim);
					constraintGraph.RemoveContact(bodyIdA, bodyIdB, colorIndex, localIndex);
				}
			}

			// Clear the smallest set bit
			bits = bits & (bits - 1);
		}
	}
}

void Physics::CollideTask(int startIndex, int endIndex, int threadIndex, StepContext& stepContext)
{
	TaskContext& taskContext = taskContexts[threadIndex];

	for (int i = startIndex; i < endIndex; ++i)
	{
		ContactSim* contactSim = stepContext.contacts[i];

		int contactId = contactSim->contactId;

		Shape& shapeA = shapes[contactSim->shapeIdA];
		Shape& shapeB = shapes[contactSim->shapeIdB];

		// Do proxies still overlap?
		bool overlap = shapeA.fatAABB.Contains(shapeB.fatAABB);
		if (overlap == false)
		{
			contactSim->simFlags |= CONTACT_SIM_FLAG_DISJOINT;
			contactSim->simFlags &= ~CONTACT_SIM_FLAG_TOUCHING;
			taskContext.contactStateBitset.SetBit(contactId);
		}
		else
		{
			bool wasTouching = (contactSim->simFlags & CONTACT_SIM_FLAG_TOUCHING);

			// Update contact respecting shape/body order (A,B)
			RigidBody2D& bodyA = rigidBodies[shapeA.bodyId];
			RigidBody2D& bodyB = rigidBodies[shapeB.bodyId];
			BodySim& bodySimA = solverSets[bodyA.setIndex].bodySims[bodyA.localIndex];
			BodySim& bodySimB = solverSets[bodyB.setIndex].bodySims[bodyB.localIndex];

			// avoid cache misses in b2PrepareContactsTask
			contactSim->bodySimIndexA = bodyA.setIndex == SET_TYPE_AWAKE ? bodyA.localIndex : NullIndex;
			contactSim->invMassA = bodySimA.invMass;
			contactSim->invIA = bodySimA.invInertia;

			contactSim->bodySimIndexB = bodyB.setIndex == SET_TYPE_AWAKE ? bodyB.localIndex : NullIndex;
			contactSim->invMassB = bodySimB.invMass;
			contactSim->invIB = bodySimB.invInertia;

			Transform2D transformA = bodySimA.transform;
			Transform2D transformB = bodySimB.transform;

			Vector2 centerOffsetA = bodySimA.localCenter * transformA.rotation;
			Vector2 centerOffsetB = bodySimB.localCenter * transformB.rotation;

			// This updates solid contacts and sensors
			bool touching = UpdateContact(*contactSim, shapeA, transformA, centerOffsetA, shapeB, transformB, centerOffsetB);

			// State changes that affect island connectivity. Also contact and sensor events.
			if (touching == true && wasTouching == false)
			{
				contactSim->simFlags |= CONTACT_SIM_FLAG_STARTED_TOUCHING;
				taskContext.contactStateBitset.SetBit(contactId);
			}
			else if (touching == false && wasTouching == true)
			{
				contactSim->simFlags |= CONTACT_SIM_FLAG_STOPPED_TOUCHING;
				taskContext.contactStateBitset.SetBit(contactId);
			}
		}
	}
}

void Physics::Solve(StepContext& stepContext)
{
	stepIndex += 1;

	MergeAwakeIslands();

	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];
	I32 awakeBodyCount = (I32)awakeSet.bodySims.Size();
	if (awakeBodyCount == 0) { return; }

	// Prepare buffers for continuous collision (fast bodies)
	stepContext.fastBodyCount = 0;
	Memory::AllocateArray(&stepContext.fastBodies, awakeBodyCount);
	stepContext.bulletBodyCount = 0;
	Memory::AllocateArray(&stepContext.bulletBodies, awakeBodyCount);

	// Solve constraints using graph coloring
	{
		GraphColor* colors = constraintGraph.colors;

		stepContext.sims = awakeSet.bodySims.Data();
		stepContext.states = awakeSet.bodyStates.Data();

		// count contacts, joints, and colors
		int awakeContactCount = 0;
		int awakeJointCount = 0;
		int activeColorCount = 0;
		for (int i = 0; i < GraphColorCount - 1; ++i)
		{
			int perColorContactCount = (I32)colors[i].contactSims.Size();
			int perColorJointCount = (I32)colors[i].jointSims.Size();
			int occupancyCount = perColorContactCount + perColorJointCount;
			activeColorCount += occupancyCount > 0 ? 1 : 0;
			awakeContactCount += perColorContactCount;
			awakeJointCount += perColorJointCount;
		}

		// Deal with void**
		{
			bodyMoveEvents.Resize(awakeBodyCount);
		}

		// Each worker receives at most M blocks of work. The workers may receive less than there is not sufficient work.
		// Each block of work has a minimum number of elements (block size). This in turn may limit number of blocks.
		// If there are many elements then the block size is increased so there are still at most M blocks of work per worker.
		// M is a tunable number that has two goals:
		// 1. keep M small to reduce overhead
		// 2. keep M large enough for other workers to be able to steal work
		// The block size is a power of two to make math efficient.

		const int blocksPerWorker = 4;
		const int maxBlockCount = blocksPerWorker * workerCount;

		// Configure blocks for tasks that parallel-for bodies
		int bodyBlockSize = 1 << 5;
		int bodyBlockCount;
		if (awakeBodyCount > bodyBlockSize * maxBlockCount)
		{
			// Too many blocks, increase block size
			bodyBlockSize = awakeBodyCount / maxBlockCount;
			bodyBlockCount = maxBlockCount;
		}
		else
		{
			bodyBlockCount = ((awakeBodyCount - 1) >> 5) + 1;
		}

		// Configure blocks for tasks parallel-for each active graph color
		// The blocks are a mix of SIMD contact blocks and joint blocks
		int activeColorIndices[GraphColorCount];

		int colorContactCounts[GraphColorCount];
		int colorContactBlockSizes[GraphColorCount];
		int colorContactBlockCounts[GraphColorCount];

		int colorJointCounts[GraphColorCount];
		int colorJointBlockSizes[GraphColorCount];
		int colorJointBlockCounts[GraphColorCount];

		int graphBlockCount = 0;

		// c is the active color index
		int simdContactCount = 0;
		int c = 0;
		for (int i = 0; i < GraphColorCount - 1; ++i)
		{
			int colorContactCount = (I32)colors[i].contactSims.Size();
			int colorJointCount = (I32)colors[i].jointSims.Size();

			if (colorContactCount + colorJointCount > 0)
			{
				activeColorIndices[c] = i;

				// 4/8-way SIMD
				int colorContactCountSIMD = colorContactCount > 0 ? ((colorContactCount - 1) >> SIMD_SHIFT) + 1 : 0;

				colorContactCounts[c] = colorContactCountSIMD;

				// determine the number of contact work blocks for this color
				if (colorContactCountSIMD > blocksPerWorker * maxBlockCount)
				{
					// too many contact blocks
					colorContactBlockSizes[c] = colorContactCountSIMD / maxBlockCount;
					colorContactBlockCounts[c] = maxBlockCount;
				}
				else if (colorContactCountSIMD > 0)
				{
					// dividing by blocksPerWorker (4)
					colorContactBlockSizes[c] = blocksPerWorker;
					colorContactBlockCounts[c] = ((colorContactCountSIMD - 1) >> 2) + 1;
				}
				else
				{
					// no contacts in this color
					colorContactBlockSizes[c] = 0;
					colorContactBlockCounts[c] = 0;
				}

				colorJointCounts[c] = colorJointCount;

				// determine number of joint work blocks for this color
				if (colorJointCount > blocksPerWorker * maxBlockCount)
				{
					// too many joint blocks
					colorJointBlockSizes[c] = colorJointCount / maxBlockCount;
					colorJointBlockCounts[c] = maxBlockCount;
				}
				else if (colorJointCount > 0)
				{
					// dividing by blocksPerWorker (4)
					colorJointBlockSizes[c] = blocksPerWorker;
					colorJointBlockCounts[c] = ((colorJointCount - 1) >> 2) + 1;
				}
				else
				{
					colorJointBlockSizes[c] = 0;
					colorJointBlockCounts[c] = 0;
				}

				graphBlockCount += colorContactBlockCounts[c] + colorJointBlockCounts[c];
				simdContactCount += colorContactCountSIMD;
				c += 1;
			}
		}
		activeColorCount = c;

		// Gather contact pointers for easy parallel-for traversal. Some may be NULL due to SIMD remainders.
		Vector<ContactSim*> contacts(NH_SIMD_WIDTH * simdContactCount);

		// Gather joint pointers for easy parallel-for traversal.
		JointSim** joints;
		Memory::AllocateArray(&joints, awakeJointCount);

		ContactConstraintSIMD* simdContactConstraints;
		Memory::AllocateArray(&simdContactConstraints, simdContactCount);

		int overflowContactCount = (I32)colors[OverflowIndex].contactSims.Size();
		ContactConstraint* overflowContactConstraints;
		Memory::AllocateArray(&overflowContactConstraints, overflowContactCount);

		constraintGraph.colors[OverflowIndex].overflowConstraints = overflowContactConstraints;

		// Distribute transient constraints to each graph color and build flat arrays of contact and joint pointers
		{
			int contactBase = 0;
			int jointBase = 0;
			for (int i = 0; i < activeColorCount; ++i)
			{
				int j = activeColorIndices[i];
				GraphColor* color = colors + j;

				int colorContactCount = (I32)color->contactSims.Size();

				if (colorContactCount == 0)
				{
					color->simdConstraints = NULL;
				}
				else
				{
					color->simdConstraints = simdContactConstraints + contactBase;

					for (int k = 0; k < colorContactCount; ++k)
					{
						contacts[NH_SIMD_WIDTH * contactBase + k] = color->contactSims.Data() + k;
					}

					// remainder
					int colorContactCountSIMD = ((colorContactCount - 1) >> SIMD_SHIFT) + 1;
					for (int k = colorContactCount; k < NH_SIMD_WIDTH * colorContactCountSIMD; ++k)
					{
						contacts[NH_SIMD_WIDTH * contactBase + k] = NULL;
					}

					contactBase += colorContactCountSIMD;
				}

				int colorJointCount = (I32)color->jointSims.Size();
				for (int k = 0; k < colorJointCount; ++k)
				{
					joints[jointBase + k] = color->jointSims.Data() + k;
				}
				jointBase += colorJointCount;
			}
		}

		// Define work blocks for preparing contacts and storing contact impulses
		int contactBlockSize = blocksPerWorker;
		int contactBlockCount = simdContactCount > 0 ? ((simdContactCount - 1) >> 2) + 1 : 0;
		if (simdContactCount > contactBlockSize * maxBlockCount)
		{
			// Too many blocks, increase block size
			contactBlockSize = simdContactCount / maxBlockCount;
			contactBlockCount = maxBlockCount;
		}

		// Define work blocks for preparing joints
		int jointBlockSize = blocksPerWorker;
		int jointBlockCount = awakeJointCount > 0 ? ((awakeJointCount - 1) >> 2) + 1 : 0;
		if (awakeJointCount > jointBlockSize * maxBlockCount)
		{
			// Too many blocks, increase block size
			jointBlockSize = awakeJointCount / maxBlockCount;
			jointBlockCount = maxBlockCount;
		}

		int stageCount = 0;

		// b2_stagePrepareJoints
		stageCount += 1;
		// b2_stagePrepareContacts
		stageCount += 1;
		// b2_stageIntegrateVelocities
		stageCount += 1;
		// b2_stageWarmStart
		stageCount += activeColorCount;
		// b2_stageSolve
		stageCount += activeColorCount;
		// b2_stageIntegratePositions
		stageCount += 1;
		// b2_stageRelax
		stageCount += activeColorCount;
		// b2_stageRestitution
		stageCount += activeColorCount;
		// b2_stageStoreImpulses
		stageCount += 1;

		SolverStage* stages;
		Memory::AllocateArray(&stages, stageCount);
		SolverBlock* bodyBlocks;
		Memory::AllocateArray(&bodyBlocks, bodyBlockCount);
		SolverBlock* contactBlocks;
		Memory::AllocateArray(&contactBlocks, contactBlockCount);
		SolverBlock* jointBlocks;
		Memory::AllocateArray(&jointBlocks, jointBlockCount);
		SolverBlock* graphBlocks;
		Memory::AllocateArray(&graphBlocks, graphBlockCount);

		// Split an awake island. This modifies:
		// - stack allocator
		// - world island array and solver set
		// - island indices on bodies, contacts, and joints
		// I'm squeezing this task in here because it may be expensive and this is a safe place to put it.
		// Note: cannot split islands in parallel with FinalizeBodies
		if (splitIslandId != NullIndex)
		{
			SplitIsland(splitIslandId);
			taskCount += 1;
		}

		// Prepare body work blocks
		for (int i = 0; i < bodyBlockCount; ++i)
		{
			SolverBlock* block = bodyBlocks + i;
			block->startIndex = i * bodyBlockSize;
			block->count = (I16)bodyBlockSize;
			block->blockType = SOLVER_BLOCK_TYPE_BODY;
			block->syncIndex = 0;
		}
		bodyBlocks[bodyBlockCount - 1].count = (I16)(awakeBodyCount - (bodyBlockCount - 1) * bodyBlockSize);

		// Prepare joint work blocks
		for (int i = 0; i < jointBlockCount; ++i)
		{
			SolverBlock* block = jointBlocks + i;
			block->startIndex = i * jointBlockSize;
			block->count = (I16)jointBlockSize;
			block->blockType = SOLVER_BLOCK_TYPE_JOINT;
			block->syncIndex = 0;
		}

		if (jointBlockCount > 0)
		{
			jointBlocks[jointBlockCount - 1].count = (I16)(awakeJointCount - (jointBlockCount - 1) * jointBlockSize);
		}

		// Prepare contact work blocks
		for (int i = 0; i < contactBlockCount; ++i)
		{
			SolverBlock* block = contactBlocks + i;
			block->startIndex = i * contactBlockSize;
			block->count = (I16)contactBlockSize;
			block->blockType = SOLVER_BLOCK_TYPE_CONTACT;
			block->syncIndex = 0;
		}

		if (contactBlockCount > 0)
		{
			contactBlocks[contactBlockCount - 1].count =
				(I16)(simdContactCount - (contactBlockCount - 1) * contactBlockSize);
		}

		// Prepare graph work blocks
		SolverBlock* graphColorBlocks[GraphColorCount];
		SolverBlock* baseGraphBlock = graphBlocks;

		for (int i = 0; i < activeColorCount; ++i)
		{
			graphColorBlocks[i] = baseGraphBlock;

			int colorJointBlockCount = colorJointBlockCounts[i];
			int colorJointBlockSize = colorJointBlockSizes[i];
			for (int j = 0; j < colorJointBlockCount; ++j)
			{
				SolverBlock* block = baseGraphBlock + j;
				block->startIndex = j * colorJointBlockSize;
				block->count = (I16)colorJointBlockSize;
				block->blockType = SOLVER_BLOCK_TYPE_GRAPH_JOINT;
				block->syncIndex = 0;
			}

			if (colorJointBlockCount > 0)
			{
				baseGraphBlock[colorJointBlockCount - 1].count =
					(I16)(colorJointCounts[i] - (colorJointBlockCount - 1) * colorJointBlockSize);
				baseGraphBlock += colorJointBlockCount;
			}

			int colorContactBlockCount = colorContactBlockCounts[i];
			int colorContactBlockSize = colorContactBlockSizes[i];
			for (int j = 0; j < colorContactBlockCount; ++j)
			{
				SolverBlock* block = baseGraphBlock + j;
				block->startIndex = j * colorContactBlockSize;
				block->count = (I16)colorContactBlockSize;
				block->blockType = SOLVER_BLOCK_TYPE_GRAPH_CONTACT;
				block->syncIndex = 0;
			}

			if (colorContactBlockCount > 0)
			{
				baseGraphBlock[colorContactBlockCount - 1].count =
					(I16)(colorContactCounts[i] - (colorContactBlockCount - 1) * colorContactBlockSize);
				baseGraphBlock += colorContactBlockCount;
			}
		}

		ptrdiff_t blockDiff = baseGraphBlock - graphBlocks;

		SolverStage* stage = stages;

		// Prepare joints
		stage->type = SOLVER_STAGE_PREPARE_JOINTS;
		stage->blocks = jointBlocks;
		stage->blockCount = jointBlockCount;
		stage->colorIndex = -1;
		stage->completionCount = 0;
		stage += 1;

		// Prepare contacts
		stage->type = SOLVER_STAGE_PREPARE_CONTACTS;
		stage->blocks = contactBlocks;
		stage->blockCount = contactBlockCount;
		stage->colorIndex = -1;
		stage->completionCount = 0;
		stage += 1;

		// Integrate velocities
		stage->type = SOLVER_STAGE_INTEGRATE_VELOCITIES;
		stage->blocks = bodyBlocks;
		stage->blockCount = bodyBlockCount;
		stage->colorIndex = -1;
		stage->completionCount = 0;
		stage += 1;

		// Warm start
		for (int i = 0; i < activeColorCount; ++i)
		{
			stage->type = SOLVER_STAGE_WARM_START;
			stage->blocks = graphColorBlocks[i];
			stage->blockCount = colorJointBlockCounts[i] + colorContactBlockCounts[i];
			stage->colorIndex = activeColorIndices[i];
			stage->completionCount = 0;
			stage += 1;
		}

		// Solve graph
		for (int i = 0; i < activeColorCount; ++i)
		{
			stage->type = SOLVER_STAGE_SOLVE;
			stage->blocks = graphColorBlocks[i];
			stage->blockCount = colorJointBlockCounts[i] + colorContactBlockCounts[i];
			stage->colorIndex = activeColorIndices[i];
			stage->completionCount = 0;
			stage += 1;
		}

		// Integrate positions
		stage->type = SOLVER_STAGE_INTEGRATE_POSITIONS;
		stage->blocks = bodyBlocks;
		stage->blockCount = bodyBlockCount;
		stage->colorIndex = -1;
		stage->completionCount = 0;
		stage += 1;

		// Relax constraints
		for (int i = 0; i < activeColorCount; ++i)
		{
			stage->type = SOLVER_STAGE_RELAX;
			stage->blocks = graphColorBlocks[i];
			stage->blockCount = colorJointBlockCounts[i] + colorContactBlockCounts[i];
			stage->colorIndex = activeColorIndices[i];
			stage->completionCount = 0;
			stage += 1;
		}

		// Restitution
		// Note: joint blocks mixed in, could have joint limit restitution
		for (int i = 0; i < activeColorCount; ++i)
		{
			stage->type = SOLVER_STAGE_RESTITUTION;
			stage->blocks = graphColorBlocks[i];
			stage->blockCount = colorJointBlockCounts[i] + colorContactBlockCounts[i];
			stage->colorIndex = activeColorIndices[i];
			stage->completionCount = 0;
			stage += 1;
		}

		// Store impulses
		stage->type = SOLVER_STAGE_STORE_IMPULSES;
		stage->blocks = contactBlocks;
		stage->blockCount = contactBlockCount;
		stage->colorIndex = -1;
		stage->completionCount = 0;
		stage += 1;

		WorkerContext workerContext[MaxWorkers];

		stepContext.graph = &constraintGraph;
		stepContext.joints = joints;
		stepContext.contacts = contacts;
		stepContext.simdContactConstraints = simdContactConstraints;
		stepContext.activeColorCount = activeColorCount;
		stepContext.workerCount = workerCount;
		stepContext.stageCount = stageCount;
		stepContext.stages = stages;
		stepContext.atomicSyncBits = 0;

		// Must use worker index because thread 0 can be assigned multiple tasks by enkiTS
		for (int i = 0; i < workerCount; ++i)
		{
			workerContext[i].context = &stepContext;
			workerContext[i].workerIndex = i;
			workerContext[i].userTask = nullptr;
			SolverTask(workerContext[i]);
			taskCount += 1;
		}

		splitIslandId = NullIndex;

		// Prepare contact, enlarged body, and island bit sets used in body finalization.
		int awakeIslandCount = (I32)awakeSet.islandSims.Size();
		for (int i = 0; i < workerCount; ++i)
		{
			TaskContext* taskContext = taskContexts.Data() + i;
			taskContext->enlargedSimBitset.SetBitCountAndClear(awakeBodyCount);
			taskContext->awakeIslandBitset.SetBitCountAndClear(awakeIslandCount);
			taskContext->splitIslandId = NullIndex;
			taskContext->splitSleepTime = 0.0f;
		}

		// Finalize bodies. Must happen after the constraint solver and after island splitting.
		FinalizeBodiesTask(0, awakeBodyCount, 0, stepContext);
		taskCount += 1;

		Memory::Free(&graphBlocks);
		Memory::Free(&jointBlocks);
		Memory::Free(&contactBlocks);
		Memory::Free(&bodyBlocks);
		Memory::Free(&stages);
		Memory::Free(&overflowContactConstraints);
		Memory::Free(&simdContactConstraints);
		Memory::Free(&joints);
	}

	// Report hit events
	// todo perhaps optimize this with a bitset
	{
		float threshold = hitEventThreshold;
		GraphColor* colors = constraintGraph.colors;
		for (int i = 0; i < GraphColorCount; ++i)
		{
			GraphColor* color = colors + i;
			int contactCount = (I32)color->contactSims.Size();
			ContactSim* contactSims = color->contactSims.Data();
			for (int j = 0; j < contactCount; ++j)
			{
				ContactSim* contactSim = contactSims + j;
				if ((contactSim->simFlags & CONTACT_SIM_FLAG_ENABLE_HIT_EVENT) == 0)
				{
					continue;
				}

				ContactHitEvent event = { 0 };
				event.approachSpeed = threshold;

				bool hit = false;
				int pointCount = contactSim->manifold.pointCount;
				for (int k = 0; k < pointCount; ++k)
				{
					ManifoldPoint* mp = contactSim->manifold.points + k;
					float approachSpeed = -mp->normalVelocity;

					// Need to check max impulse because the point may be speculative and not colliding
					if (approachSpeed > event.approachSpeed && mp->maxNormalImpulse > 0.0f)
					{
						event.approachSpeed = approachSpeed;
						event.point = mp->point;
						hit = true;
					}
				}

				if (hit == true)
				{
					event.normal = contactSim->manifold.normal;

					Shape& shapeA = shapes[contactSim->shapeIdA];
					Shape& shapeB = shapes[contactSim->shapeIdB];

					event.shapeIdA = shapeA.id + 1;
					event.shapeIdB = shapeB.id + 1;

					contactHitEvents.Push(event);
				}
			}
		}
	}

	// Gather bits for all sim bodies that have enlarged AABBs
	Bitset& simBitset = taskContexts[0].enlargedSimBitset;
	for (int i = 1; i < workerCount; ++i)
	{
		simBitset.InPlaceUnion(taskContexts[i].enlargedSimBitset);
	}

	// Enlarge broad-phase proxies and build move array
	// Apply shape AABB changes to broad-phase. This also create the move array which must be
	// in deterministic order. I'm tracking sim bodies because the number of shape ids can be huge.
	{
		U32 wordCount = (U32)simBitset.blockCount;
		U64* bits = simBitset.bits;

		// Fast array access is important here
		BodySim* bodySimArray = awakeSet.bodySims.Data();
		Shape* shapeArray = shapes.Data();

		for (U32 k = 0; k < wordCount; ++k)
		{
			U64 word = bits[k];
			while (word != 0)
			{
				UL32 ctz = _BitScanForward64(&ctz, word);
				U32 bodySimIndex = 64 * k + ctz;

				BodySim* bodySim = bodySimArray + bodySimIndex;
				RigidBody2D& body = rigidBodies[bodySim->bodyId];

				int shapeId = body.headShapeId;
				while (shapeId != NullIndex)
				{
					Shape* shape = shapeArray + shapeId;

					if (shape->enlargedAABB)
					{
						Broadphase::EnlargeProxy(shape->proxyKey, shape->fatAABB);
						shape->enlargedAABB = false;
					}
					else if (shape->isFast)
					{
						// Shape is fast. It's aabb will be enlarged in continuous collision.
						Broadphase::BufferMove(shape->proxyKey);
					}

					shapeId = shape->nextShapeId;
				}

				// Clear the smallest set bit
				word = word & (word - 1);
			}
		}
	}

	// Parallel continuous collision
	if (stepContext.fastBodyCount > 0)
	{
		// fast bodies
		int minRange = 8;
		FastBodyTask(0, stepContext.fastBodyCount, 0, stepContext);
		taskCount += 1;
	}

	// Serially enlarge broad-phase proxies for fast shapes
	// Doing this here so that bullet shapes see them
	{
		DynamicTree& dynamicTree = Broadphase::trees[BODY_TYPE_DYNAMIC];

		// Fast array access is important here
		BodySim* bodySimArray = awakeSet.bodySims.Data();
		Shape* shapeArray = shapes.Data();

		int* fastBodySimIndices = stepContext.fastBodies;
		int fastBodyCount = stepContext.fastBodyCount;

		// This loop has non-deterministic order but it shouldn't affect the result
		for (int i = 0; i < fastBodyCount; ++i)
		{
			BodySim* fastBodySim = bodySimArray + fastBodySimIndices[i];
			if (fastBodySim->enlargeAABB == false) { continue; }

			// clear flag
			fastBodySim->enlargeAABB = false;

			int bodyId = fastBodySim->bodyId;

			RigidBody2D& fastBody = rigidBodies[bodyId];

			int shapeId = fastBody.headShapeId;
			while (shapeId != NullIndex)
			{
				Shape* shape = shapeArray + shapeId;
				if (shape->enlargedAABB == false)
				{
					shapeId = shape->nextShapeId;
					continue;
				}

				// clear flag
				shape->enlargedAABB = false;

				int proxyKey = shape->proxyKey;
				int proxyId = PROXY_ID(proxyKey);

				dynamicTree.EnlargeProxy(proxyId, shape->fatAABB);

				shapeId = shape->nextShapeId;
			}
		}
	}

	if (stepContext.bulletBodyCount > 0)
	{
		// bullet bodies
		int minRange = 8;
		BulletBodyTask(0, stepContext.bulletBodyCount, 0, stepContext);
		taskCount += 1;
	}

	// Serially enlarge broad-phase proxies for bullet shapes
	{
		DynamicTree& dynamicTree = Broadphase::trees[BODY_TYPE_DYNAMIC];

		// Fast array access is important here
		BodySim* bodySimArray = awakeSet.bodySims.Data();
		Shape* shapeArray = shapes.Data();

		// Serially enlarge broad-phase proxies for bullet shapes
		int* bulletBodySimIndices = stepContext.bulletBodies;
		int bulletBodyCount = stepContext.bulletBodyCount;

		// This loop has non-deterministic order but it shouldn't affect the result
		for (int i = 0; i < bulletBodyCount; ++i)
		{
			BodySim* bulletBodySim = bodySimArray + bulletBodySimIndices[i];
			if (bulletBodySim->enlargeAABB == false)
			{
				continue;
			}

			// clear flag
			bulletBodySim->enlargeAABB = false;

			int bodyId = bulletBodySim->bodyId;
			RigidBody2D& bulletBody = rigidBodies[bodyId];

			int shapeId = bulletBody.headShapeId;
			while (shapeId != NullIndex)
			{
				Shape* shape = shapeArray + shapeId;
				if (shape->enlargedAABB == false)
				{
					shapeId = shape->nextShapeId;
					continue;
				}

				// clear flag
				shape->enlargedAABB = false;

				int proxyKey = shape->proxyKey;
				int proxyId = PROXY_ID(proxyKey);

				dynamicTree.EnlargeProxy(proxyId, shape->fatAABB);

				shapeId = shape->nextShapeId;
			}
		}
	}

	Memory::Free(&stepContext.bulletBodies);
	stepContext.bulletBodies = NULL;
	stepContext.bulletBodyCount = 0;

	Memory::Free(&stepContext.fastBodies);
	stepContext.fastBodies = NULL;
	stepContext.fastBodyCount = 0;

	// Island sleeping
	// This must be done last because putting islands to sleep invalidates the enlarged body bits.
	if (enableSleep)
	{
		// Collect split island candidate for the next time step. No need to split if sleeping is disabled.
		float splitSleepTimer = 0.0f;
		for (int i = 0; i < workerCount; ++i)
		{
			TaskContext* taskContext = taskContexts.Data() + i;
			if (taskContext->splitIslandId != NullIndex && taskContext->splitSleepTime >= splitSleepTimer)
			{
				// Tie breaking for determinism. Largest island id wins. Needed due to work stealing.
				if (taskContext->splitSleepTime == splitSleepTimer && taskContext->splitIslandId < splitIslandId)
				{
					continue;
				}

				splitIslandId = taskContext->splitIslandId;
				splitSleepTimer = taskContext->splitSleepTime;
			}
		}

		Bitset& awakeIslandBitset = taskContexts[0].awakeIslandBitset;
		for (int i = 1; i < workerCount; ++i)
		{
			awakeIslandBitset.InPlaceUnion(taskContexts[i].awakeIslandBitset);
		}

		// Need to process in reverse because this moves islands to sleeping solver sets.
		IslandSim* islands = awakeSet.islandSims.Data();
		int count = (I32)awakeSet.islandSims.Size();
		for (int islandIndex = count - 1; islandIndex >= 0; islandIndex -= 1)
		{
			if (awakeIslandBitset.GetBit(islandIndex) == true) { continue; }

			IslandSim* island = islands + islandIndex;
			int islandId = island->islandId;

			TrySleepIsland(islandId);
		}
	}
}

void Physics::SolverTask(WorkerContext& workerContext)
{
	int workerIndex = workerContext.workerIndex;
	StepContext* context = workerContext.context;
	int activeColorCount = context->activeColorCount;
	SolverStage* stages = context->stages;

	if (workerIndex == 0)
	{
		// Main thread synchronizes the workers and does work itself.
		//
		// Stages are re-used by loops so that I don't need more stages for large iteration counts.
		// The sync indices grow monotonically for the body/graph/constraint groupings because they share solver blocks.
		// The stage index and sync indices are combined in to sync bits for atomic synchronization.
		// The workers need to compute the previous sync index for a given stage so that CAS works correctly. This
		// setup makes this easy to do.

		/*
		b2_stagePrepareJoints,
		b2_stagePrepareContacts,
		b2_stageIntegrateVelocities,
		b2_stageWarmStart,
		b2_stageSolve,
		b2_stageIntegratePositions,
		b2_stageRelax,
		b2_stageRestitution,
		b2_stageStoreImpulses
		*/

		int bodySyncIndex = 1;
		int stageIndex = 0;

		// This stage loops over all awake joints
		U32 jointSyncIndex = 1;
		U32 syncBits = (jointSyncIndex << 16) | stageIndex;
		ExecuteMainStage(stages[stageIndex], context, syncBits);
		stageIndex += 1;
		jointSyncIndex += 1;

		// This stage loops over all contact constraints
		U32 contactSyncIndex = 1;
		syncBits = (contactSyncIndex << 16) | stageIndex;
		ExecuteMainStage(stages[stageIndex], context, syncBits);
		stageIndex += 1;
		contactSyncIndex += 1;

		int graphSyncIndex = 1;

		// Single-threaded overflow work. These constraints don't fit in the graph coloring.
		PrepareOverflowJoints(*context);
		PrepareOverflowContacts(*context);

		int subStepCount = context->subStepCount;
		for (int i = 0; i < subStepCount; ++i)
		{
			// stage index restarted each iteration
			// syncBits still increases monotonically because the upper bits increase each iteration
			int iterStageIndex = stageIndex;

			// integrate velocities
			syncBits = (bodySyncIndex << 16) | iterStageIndex;
			ExecuteMainStage(stages[iterStageIndex], context, syncBits);
			iterStageIndex += 1;
			bodySyncIndex += 1;

			// warm start constraints
			WarmStartOverflowJoints(*context);
			WarmStartOverflowContacts(*context);

			for (int colorIndex = 0; colorIndex < activeColorCount; ++colorIndex)
			{
				syncBits = (graphSyncIndex << 16) | iterStageIndex;
				ExecuteMainStage(stages[iterStageIndex], context, syncBits);
				iterStageIndex += 1;
			}
			graphSyncIndex += 1;

			// solve constraints
			bool useBias = true;
			SolveOverflowJoints(*context, useBias);
			SolveOverflowContacts(*context, useBias);

			for (int colorIndex = 0; colorIndex < activeColorCount; ++colorIndex)
			{
				syncBits = (graphSyncIndex << 16) | iterStageIndex;
				ExecuteMainStage(stages[iterStageIndex], context, syncBits);
				iterStageIndex += 1;
			}
			graphSyncIndex += 1;

			// integrate positions
			syncBits = (bodySyncIndex << 16) | iterStageIndex;
			ExecuteMainStage(stages[iterStageIndex], context, syncBits);
			iterStageIndex += 1;
			bodySyncIndex += 1;

			// relax constraints
			useBias = false;
			SolveOverflowJoints(*context, useBias);
			SolveOverflowContacts(*context, useBias);

			for (int colorIndex = 0; colorIndex < activeColorCount; ++colorIndex)
			{
				syncBits = (graphSyncIndex << 16) | iterStageIndex;
				ExecuteMainStage(stages[iterStageIndex], context, syncBits);
				iterStageIndex += 1;
			}
			graphSyncIndex += 1;
		}

		// advance the stage according to the sub-stepping tasks just completed
		// integrate velocities / warm start / solve / integrate positions / relax
		stageIndex += 1 + activeColorCount + activeColorCount + 1 + activeColorCount;

		// Restitution
		{
			ApplyOverflowRestitution(*context);

			int iterStageIndex = stageIndex;
			for (int colorIndex = 0; colorIndex < activeColorCount; ++colorIndex)
			{
				syncBits = (graphSyncIndex << 16) | iterStageIndex;
				ExecuteMainStage(stages[iterStageIndex], context, syncBits);
				iterStageIndex += 1;
			}
			// graphSyncIndex += 1;
			stageIndex += activeColorCount;
		}

		StoreOverflowImpulses(*context);

		syncBits = (contactSyncIndex << 16) | stageIndex;
		ExecuteMainStage(stages[stageIndex], context, syncBits);

		// Signal workers to finish
		context->atomicSyncBits.store(U32_MAX);

		return;
	}

	// Worker spins and waits for work
	U32 lastSyncBits = 0;
	// uint64_t maxSpinTime = 10;
	while (true)
	{
		// Spin until main thread bumps changes the sync bits. This can waste significant time overall, but it is necessary for
		// parallel simulation with graph coloring.
		U32 syncBits;
		int spinCount = 0;
		while ((syncBits = context->atomicSyncBits.load()) == lastSyncBits)
		{
			if (spinCount > 5)
			{
				YieldThread();
				spinCount = 0;
			}
			else
			{
				// Using the cycle counter helps to account for variation in mm_pause timing across different
				// CPUs. However, this is X64 only.
				// uint64_t prev = __rdtsc();
				// do
				//{
				//	Pause();
				//}
				// while ((__rdtsc() - prev) < maxSpinTime);
				// maxSpinTime += 10;
				Pause();
				Pause();
				spinCount += 1;
			}
		}

		if (syncBits == U32_MAX)
		{
			// sentinel hit
			break;
		}

		int stageIndex = syncBits & 0xFFFF;

		int syncIndex = (syncBits >> 16) & 0xFFFF;

		int previousSyncIndex = syncIndex - 1;

		ExecuteStage(stages[stageIndex], context, previousSyncIndex, syncIndex, workerIndex);

		lastSyncBits = syncBits;
	}
}

void Physics::ExecuteStage(SolverStage& stage, StepContext* context, int previousSyncIndex, int syncIndex, int workerIndex)
{
	int completedCount = 0;
	SolverBlock* blocks = stage.blocks;
	int blockCount = stage.blockCount;

	int expectedSyncIndex = previousSyncIndex;

	int startIndex = GetWorkerStartIndex(workerIndex, blockCount, context->workerCount);
	if (startIndex == NullIndex) { return; }

	int blockIndex = startIndex;

	// Caution: this can change expectedSyncIndex
	while (blocks[blockIndex].syncIndex.compare_exchange_strong(expectedSyncIndex, syncIndex) == true)
	{
		ExecuteBlock(stage, *context, blocks[blockIndex]);

		completedCount += 1;
		blockIndex += 1;
		if (blockIndex >= blockCount) { blockIndex = 0; }

		expectedSyncIndex = previousSyncIndex;
	}

	// Search backwards for blocks
	blockIndex = startIndex - 1;
	while (true)
	{
		if (blockIndex < 0)
		{
			blockIndex = blockCount - 1;
		}

		expectedSyncIndex = previousSyncIndex;

		// Caution: this can change expectedSyncIndex
		if (blocks[blockIndex].syncIndex.compare_exchange_strong(expectedSyncIndex, syncIndex) == false)
		{
			break;
		}

		ExecuteBlock(stage, *context, blocks[blockIndex]);
		completedCount += 1;
		blockIndex -= 1;
	}

	stage.completionCount.fetch_add(completedCount);
}

void Physics::ExecuteMainStage(SolverStage& stage, StepContext* context, U32 syncBits)
{
	int blockCount = stage.blockCount;
	if (blockCount == 0) { return; }

	if (blockCount == 1)
	{
		ExecuteBlock(stage, *context, *stage.blocks);
	}
	else
	{
		context->atomicSyncBits.store(syncBits);

		int syncIndex = (syncBits >> 16) & 0xFFFF;
		int previousSyncIndex = syncIndex - 1;

		ExecuteStage(stage, context, previousSyncIndex, syncIndex, 0);

		// todo consider using the cycle counter as well
		while (stage.completionCount.load() != blockCount)
		{
			Pause();
		}

		stage.completionCount.store(0);
	}
}

void Physics::ExecuteBlock(SolverStage& stage, StepContext& context, SolverBlock& block)
{
	SolverStageType stageType = stage.type;
	SolverBlockType blockType = (SolverBlockType)block.blockType;
	int startIndex = block.startIndex;
	int endIndex = startIndex + block.count;

	switch (stageType)
	{
	case SOLVER_STAGE_PREPARE_JOINTS: { PrepareJointsTask(startIndex, endIndex, context); } break;
	case SOLVER_STAGE_PREPARE_CONTACTS: { PrepareContactsTask(startIndex, endIndex, context); } break;
	case SOLVER_STAGE_INTEGRATE_VELOCITIES: { IntegrateVelocitiesTask(startIndex, endIndex, context); } break;
	case SOLVER_STAGE_WARM_START: {
		if (enableWarmStarting)
		{
			if (blockType == SOLVER_BLOCK_TYPE_CONTACT) { WarmStartContactsTask(startIndex, endIndex, context, stage.colorIndex); }
			else if (blockType == SOLVER_BLOCK_TYPE_JOINT) { WarmStartJointsTask(startIndex, endIndex, context, stage.colorIndex); }
		}
	} break;
	case SOLVER_STAGE_SOLVE: {
		if (blockType == SOLVER_BLOCK_TYPE_CONTACT) { SolveContactsTask(startIndex, endIndex, context, stage.colorIndex, true); }
		else if (blockType == SOLVER_BLOCK_TYPE_JOINT) { SolveJointsTask(startIndex, endIndex, context, stage.colorIndex, true); }
	} break;
	case SOLVER_STAGE_INTEGRATE_POSITIONS: { IntegratePositionsTask(startIndex, endIndex, context); } break;
	case SOLVER_STAGE_RELAX: {
		if (blockType == SOLVER_BLOCK_TYPE_CONTACT) { SolveContactsTask(startIndex, endIndex, context, stage.colorIndex, false); }
		else if (blockType == SOLVER_BLOCK_TYPE_JOINT) { SolveJointsTask(startIndex, endIndex, context, stage.colorIndex, false); }
	} break;

	case SOLVER_STAGE_RESTITUTION: {
		if (blockType == SOLVER_BLOCK_TYPE_CONTACT) { ApplyRestitutionTask(startIndex, endIndex, context, stage.colorIndex); }
	} break;

	case SOLVER_STAGE_STORE_IMPULSES: { StoreImpulsesTask(startIndex, endIndex, context); } break;
	}
}

int Physics::GetWorkerStartIndex(int workerIndex, int blockCount, int workerCount)
{
	if (blockCount <= workerCount)
	{
		return workerIndex < blockCount ? workerIndex : NullIndex;
	}

	int blocksPerWorker = blockCount / workerCount;
	int remainder = blockCount - blocksPerWorker * workerCount;
	return blocksPerWorker * workerIndex + Math::Min(remainder, workerIndex);
}

void Physics::WarmStartOverflowContacts(StepContext& context)
{
	GraphColor* color = constraintGraph.colors + OverflowIndex;
	ContactConstraint* constraints = color->overflowConstraints;
	int contactCount = (I32)color->contactSims.Size();
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];
	BodyState* states = awakeSet.bodyStates.Data();

	// This is a dummy state to represent a static body because static bodies don't have a solver body.
	BodyState dummyState = BodyStateIdentity;

	for (int i = 0; i < contactCount; ++i)
	{
		const ContactConstraint& constraint = constraints[i];

		int indexA = constraint.indexA;
		int indexB = constraint.indexB;

		BodyState& stateA = indexA == NullIndex ? dummyState : states[indexA];
		BodyState& stateB = indexB == NullIndex ? dummyState : states[indexB];

		Vector2 vA = stateA.linearVelocity;
		float wA = stateA.angularVelocity;
		Vector2 vB = stateB.linearVelocity;
		float wB = stateB.angularVelocity;

		float mA = constraint.invMassA;
		float iA = constraint.invIA;
		float mB = constraint.invMassB;
		float iB = constraint.invIB;

		// Stiffer for static contacts to avoid bodies getting pushed through the ground
		Vector2 normal = constraint.normal;
		Vector2 tangent = constraint.normal.PerpendicularRight();
		int pointCount = constraint.pointCount;

		for (int j = 0; j < pointCount; ++j)
		{
			const ContactConstraintPoint& cp = constraint.points[j];

			// fixed anchors
			Vector2 rA = cp.anchorA;
			Vector2 rB = cp.anchorB;

			Vector2 P = normal * cp.normalImpulse + tangent * cp.tangentImpulse;
			wA -= iA * rA.Cross(P);
			vA = vA + P * -mA;
			wB += iB * rB.Cross(P);
			vB = vB + P * mB;
		}

		stateA.linearVelocity = vA;
		stateA.angularVelocity = wA;
		stateB.linearVelocity = vB;
		stateB.angularVelocity = wB;
	}
}

void Physics::SolveOverflowContacts(StepContext& context, bool useBias)
{
	GraphColor& color = constraintGraph.colors[OverflowIndex];
	ContactConstraint* constraints = color.overflowConstraints;
	int contactCount = (I32)color.contactSims.Size();
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];
	BodyState* states = awakeSet.bodyStates.Data();

	float inv_h = context.inv_h;
	const float pushout = contactPushoutVelocity;

	// This is a dummy body to represent a static body since static bodies don't have a solver body.
	BodyState dummyState = BodyStateIdentity;

	for (int i = 0; i < contactCount; ++i)
	{
		ContactConstraint& constraint = constraints[i];
		float mA = constraint.invMassA;
		float iA = constraint.invIA;
		float mB = constraint.invMassB;
		float iB = constraint.invIB;

		BodyState& stateA = constraint.indexA == NullIndex ? dummyState : states[constraint.indexA];
		Vector2 vA = stateA.linearVelocity;
		float wA = stateA.angularVelocity;
		Quaternion2 dqA = stateA.deltaRotation;

		BodyState& stateB = constraint.indexB == NullIndex ? dummyState : states[constraint.indexB];
		Vector2 vB = stateB.linearVelocity;
		float wB = stateB.angularVelocity;
		Quaternion2 dqB = stateB.deltaRotation;

		Vector2 dp = stateB.deltaPosition - stateA.deltaPosition;

		Vector2 normal = constraint.normal;
		Vector2 tangent = normal.PerpendicularRight();
		float friction = constraint.friction;
		Softness softness = constraint.softness;

		int pointCount = constraint.pointCount;

		for (int j = 0; j < pointCount; ++j)
		{
			ContactConstraintPoint& cp = constraint.points[j];

			// compute current separation
			// this is subject to round-off error if the anchor is far from the body center of mass
			Vector2 ds = dp + (cp.anchorB * dqB - cp.anchorA * dqA);
			float s = ds.Dot(normal) + cp.baseSeparation;

			float velocityBias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;
			if (s > 0.0f)
			{
				// speculative bias
				velocityBias = s * inv_h;
			}
			else if (useBias)
			{
				velocityBias = Math::Max(softness.biasRate * s, -pushout);
				massScale = softness.massScale;
				impulseScale = softness.impulseScale;
			}

			// fixed anchor points
			Vector2 rA = cp.anchorA;
			Vector2 rB = cp.anchorB;

			// relative normal velocity at contact
			Vector2 vrA = vA + rA.CrossInv(wA);
			Vector2 vrB = vB + rB.CrossInv(wB);
			float vn = (vrB - vrA).Dot(normal);

			// incremental normal impulse
			float impulse = -cp.normalMass * massScale * (vn + velocityBias) - impulseScale * cp.normalImpulse;

			// clamp the accumulated impulse
			float newImpulse = Math::Max(cp.normalImpulse + impulse, 0.0f);
			impulse = newImpulse - cp.normalImpulse;
			cp.normalImpulse = newImpulse;
			cp.maxNormalImpulse = Math::Max(cp.maxNormalImpulse, impulse);

			// apply normal impulse
			Vector2 P = normal * impulse;
			vA = vA + P * mA;
			wA -= iA * rA.Cross(P);

			vB = vB + P * mB;
			wB += iB * rB.Cross(P);
		}

		for (int j = 0; j < pointCount; ++j)
		{
			ContactConstraintPoint& cp = constraint.points[j];

			// fixed anchor points
			Vector2 rA = cp.anchorA;
			Vector2 rB = cp.anchorB;

			// relative tangent velocity at contact
			Vector2 vrB = vB + rB.CrossInv(wB);
			Vector2 vrA = vA + rA.CrossInv(wA);
			float vt = (vrB - vrA).Dot(tangent);

			// incremental tangent impulse
			float impulse = cp.tangentMass * (-vt);

			// clamp the accumulated force
			float maxFriction = friction * cp.normalImpulse;
			float newImpulse = Math::Clamp(cp.tangentImpulse + impulse, -maxFriction, maxFriction);
			impulse = newImpulse - cp.tangentImpulse;
			cp.tangentImpulse = newImpulse;

			// apply tangent impulse
			Vector2 P = tangent * impulse;
			vA = vA - P * mA;
			wA -= iA * rA.Cross(P);
			vB = vB + P * mB;
			wB += iB * rB.Cross(P);
		}

		stateA.linearVelocity = vA;
		stateA.angularVelocity = wA;
		stateB.linearVelocity = vB;
		stateB.angularVelocity = wB;
	}
}

void Physics::ApplyOverflowRestitution(StepContext& context)
{
	GraphColor& color = constraintGraph.colors[OverflowIndex];
	ContactConstraint* constraints = color.overflowConstraints;
	int contactCount = (I32)color.contactSims.Size();
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];
	BodyState* states = awakeSet.bodyStates.Data();

	float threshold = restitutionThreshold;

	// dummy state to represent a static body
	BodyState dummyState = BodyStateIdentity;

	for (int i = 0; i < contactCount; ++i)
	{
		ContactConstraint& constraint = constraints[i];

		float restitution = constraint.restitution;
		if (restitution == 0.0f) { continue; }

		float mA = constraint.invMassA;
		float iA = constraint.invIA;
		float mB = constraint.invMassB;
		float iB = constraint.invIB;

		BodyState& stateA = constraint.indexA == NullIndex ? dummyState : states[constraint.indexA];
		Vector2 vA = stateA.linearVelocity;
		float wA = stateA.angularVelocity;

		BodyState& stateB = constraint.indexB == NullIndex ? dummyState : states[constraint.indexB];
		Vector2 vB = stateB.linearVelocity;
		float wB = stateB.angularVelocity;

		Vector2 normal = constraint.normal;
		int pointCount = constraint.pointCount;

		// it is possible to get more accurate restitution by iterating
		// this only makes a difference if there are two contact points
		// for (int iter = 0; iter < 10; ++iter)
		{
			for (int j = 0; j < pointCount; ++j)
			{
				ContactConstraintPoint& cp = constraint.points[j];

				// if the normal impulse is zero then there was no collision
				// this skips speculative contact points that didn't generate an impulse
				// The max normal impulse is used in case there was a collision that moved away within the sub-step process
				if (cp.relativeVelocity > -threshold || cp.maxNormalImpulse == 0.0f) { continue; }

				// fixed anchor points
				Vector2 rA = cp.anchorA;
				Vector2 rB = cp.anchorB;

				// relative normal velocity at contact
				Vector2 vrB = vB + rB.CrossInv(wB);
				Vector2 vrA = vA + rA.CrossInv(wA);
				float vn = (vrB - vrA).Dot(normal);

				// compute normal impulse
				float impulse = -cp.normalMass * (vn + restitution * cp.relativeVelocity);

				// clamp the accumulated impulse
				// todo should this be stored?
				float newImpulse = Math::Max(cp.normalImpulse + impulse, 0.0f);
				impulse = newImpulse - cp.normalImpulse;
				cp.normalImpulse = newImpulse;
				cp.maxNormalImpulse = Math::Max(cp.maxNormalImpulse, impulse);

				// apply contact impulse
				Vector2 P = normal * impulse;
				vA = vA - P * mA;
				wA -= iA * rA.Cross(P);
				vB = vB + P * mB;
				wB += iB * rB.Cross(P);
			}
		}

		stateA.linearVelocity = vA;
		stateA.angularVelocity = wA;
		stateB.linearVelocity = vB;
		stateB.angularVelocity = wB;
	}
}

void Physics::StoreOverflowImpulses(StepContext& context)
{
	GraphColor& color = constraintGraph.colors[OverflowIndex];
	ContactConstraint* constraints = color.overflowConstraints;
	ContactSim* contacts = color.contactSims.Data();
	int contactCount = (I32)color.contactSims.Size();

	// float hitEventThreshold = context->world->hitEventThreshold;

	for (int i = 0; i < contactCount; ++i)
	{
		const ContactConstraint& constraint = constraints[i];
		ContactSim& contact = contacts[i];
		Manifold& manifold = contact.manifold;
		int pointCount = manifold.pointCount;

		for (int j = 0; j < pointCount; ++j)
		{
			manifold.points[j].normalImpulse = constraint.points[j].normalImpulse;
			manifold.points[j].tangentImpulse = constraint.points[j].tangentImpulse;
			manifold.points[j].maxNormalImpulse = constraint.points[j].maxNormalImpulse;
			manifold.points[j].normalVelocity = constraint.points[j].relativeVelocity;
		}
	}
}

void Physics::PrepareOverflowContacts(StepContext& context)
{
	GraphColor& color = constraintGraph.colors[OverflowIndex];
	ContactConstraint* constraints = color.overflowConstraints;
	int contactCount = (I32)color.contactSims.Size();
	ContactSim* contacts = color.contactSims.Data();
	BodyState* awakeStates = context.states;

	// Stiffer for static contacts to avoid bodies getting pushed through the ground
	Softness contactSoftness = context.contactSoftness;
	Softness staticSoftness = context.staticSoftness;

	float warmStartScale = enableWarmStarting ? 1.0f : 0.0f;

	for (int i = 0; i < contactCount; ++i)
	{
		ContactSim& contactSim = contacts[i];

		const Manifold& manifold = contactSim.manifold;
		int pointCount = manifold.pointCount;

		int indexA = contactSim.bodySimIndexA;
		int indexB = contactSim.bodySimIndexB;

		ContactConstraint& constraint = constraints[i];
		constraint.indexA = indexA;
		constraint.indexB = indexB;
		constraint.normal = manifold.normal;
		constraint.friction = contactSim.friction;
		constraint.restitution = contactSim.restitution;
		constraint.pointCount = pointCount;

		Vector2 vA = Vector2Zero;
		float wA = 0.0f;
		float mA = contactSim.invMassA;
		float iA = contactSim.invIA;
		if (indexA != NullIndex)
		{
			BodyState& stateA = awakeStates[indexA];
			vA = stateA.linearVelocity;
			wA = stateA.angularVelocity;
		}

		Vector2 vB = Vector2Zero;
		float wB = 0.0f;
		float mB = contactSim.invMassB;
		float iB = contactSim.invIB;
		if (indexB != NullIndex)
		{
			BodyState& stateB = awakeStates[indexB];
			vB = stateB.linearVelocity;
			wB = stateB.angularVelocity;
		}

		if (indexA == NullIndex || indexB == NullIndex)
		{
			constraint.softness = staticSoftness;
		}
		else
		{
			constraint.softness = contactSoftness;
		}

		// copy mass into constraint to avoid cache misses during sub-stepping
		constraint.invMassA = mA;
		constraint.invIA = iA;
		constraint.invMassB = mB;
		constraint.invIB = iB;

		Vector2 normal = constraint.normal;
		Vector2 tangent = constraint.normal.PerpendicularRight();

		for (int j = 0; j < pointCount; ++j)
		{
			const ManifoldPoint& mp = manifold.points[j];
			ContactConstraintPoint& cp = constraint.points[j];

			cp.normalImpulse = warmStartScale * mp.normalImpulse;
			cp.tangentImpulse = warmStartScale * mp.tangentImpulse;
			cp.maxNormalImpulse = 0.0f;

			Vector2 rA = mp.anchorA;
			Vector2 rB = mp.anchorB;

			cp.anchorA = rA;
			cp.anchorB = rB;
			cp.baseSeparation = mp.separation - (rB - rA).Dot(normal);

			float rnA = rA.Cross(normal);
			float rnB = rB.Cross(normal);
			float kNormal = mA + mB + iA * rnA * rnA + iB * rnB * rnB;
			cp.normalMass = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

			float rtA = rA.Cross(tangent);
			float rtB = rB.Cross(tangent);
			float kTangent = mA + mB + iA * rtA * rtA + iB * rtB * rtB;
			cp.tangentMass = kTangent > 0.0f ? 1.0f / kTangent : 0.0f;

			// Save relative velocity for restitution
			Vector2 vrA = vA + rA.CrossInv(wA);
			Vector2 vrB = vB + rB.CrossInv(wB);
			cp.relativeVelocity = normal.Dot(vrB - vrA);
		}
	}
}

void Physics::PrepareContactsTask(int startIndex, int endIndex, StepContext& context)
{
	Vector<ContactSim*> contacts = context.contacts;
	ContactConstraintSIMD* constraints = context.simdContactConstraints;
	BodyState* awakeStates = context.states;

	// Stiffer for static contacts to avoid bodies getting pushed through the ground
	Softness contactSoftness = context.contactSoftness;
	Softness staticSoftness = context.staticSoftness;

	float warmStartScale = enableWarmStarting ? 1.0f : 0.0f;

	for (int i = startIndex; i < endIndex; ++i)
	{
		ContactConstraintSIMD& constraint = constraints[i];

		for (int j = 0; j < NH_SIMD_WIDTH; ++j)
		{
			ContactSim* contactSim = contacts[NH_SIMD_WIDTH * i + j];

			if (contactSim != NULL)
			{
				const Manifold& manifold = contactSim->manifold;

				int indexA = contactSim->bodySimIndexA;
				int indexB = contactSim->bodySimIndexB;

				constraint.indexA[j] = indexA;
				constraint.indexB[j] = indexB;

				Vector2 vA = Vector2Zero;
				float wA = 0.0f;
				float mA = contactSim->invMassA;
				float iA = contactSim->invIA;
				if (indexA != NullIndex)
				{
					BodyState& stateA = awakeStates[indexA];
					vA = stateA.linearVelocity;
					wA = stateA.angularVelocity;
				}

				Vector2 vB = Vector2Zero;
				float wB = 0.0f;
				float mB = contactSim->invMassB;
				float iB = contactSim->invIB;
				if (indexB != NullIndex)
				{
					BodyState& stateB = awakeStates[indexB];
					vB = stateB.linearVelocity;
					wB = stateB.angularVelocity;
				}

				((float*)&constraint.invMassA)[j] = mA;
				((float*)&constraint.invMassB)[j] = mB;
				((float*)&constraint.invIA)[j] = iA;
				((float*)&constraint.invIB)[j] = iB;

				Softness soft = (indexA == NullIndex || indexB == NullIndex) ? staticSoftness : contactSoftness;

				Vector2 normal = manifold.normal;
				((float*)&constraint.normal.x)[j] = normal.x;
				((float*)&constraint.normal.y)[j] = normal.y;

				((float*)&constraint.friction)[j] = contactSim->friction;
				((float*)&constraint.restitution)[j] = contactSim->restitution;
				((float*)&constraint.biasRate)[j] = soft.biasRate;
				((float*)&constraint.massScale)[j] = soft.massScale;
				((float*)&constraint.impulseScale)[j] = soft.impulseScale;

				Vector2 tangent = normal.PerpendicularRight();

				{
					const ManifoldPoint& mp = manifold.points[0];

					Vector2 rA = mp.anchorA;
					Vector2 rB = mp.anchorB;

					((float*)&constraint.anchorA1.x)[j] = rA.x;
					((float*)&constraint.anchorA1.y)[j] = rA.y;
					((float*)&constraint.anchorB1.x)[j] = rB.x;
					((float*)&constraint.anchorB1.y)[j] = rB.y;

					((float*)&constraint.baseSeparation1)[j] = mp.separation - (rB - rA).Dot(normal);

					((float*)&constraint.normalImpulse1)[j] = warmStartScale * mp.normalImpulse;
					((float*)&constraint.tangentImpulse1)[j] = warmStartScale * mp.tangentImpulse;
					((float*)&constraint.maxNormalImpulse1)[j] = 0.0f;

					float rnA = rA.Cross(normal);
					float rnB = rB.Cross(normal);
					float kNormal = mA + mB + iA * rnA * rnA + iB * rnB * rnB;
					((float*)&constraint.normalMass1)[j] = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

					float rtA = rA.Cross(tangent);
					float rtB = rB.Cross(tangent);
					float kTangent = mA + mB + iA * rtA * rtA + iB * rtB * rtB;
					((float*)&constraint.tangentMass1)[j] = kTangent > 0.0f ? 1.0f / kTangent : 0.0f;

					// relative velocity for restitution
					Vector2 vrA = vA + rA.CrossInv(wA);
					Vector2 vrB = vB + rB.CrossInv(wB);
					((float*)&constraint.relativeVelocity1)[j] = normal.Dot(vrB - vrA);
				}

				int pointCount = manifold.pointCount;

				if (pointCount == 2)
				{
					const ManifoldPoint& mp = manifold.points[1];

					Vector2 rA = mp.anchorA;
					Vector2 rB = mp.anchorB;

					((float*)&constraint.anchorA2.x)[j] = rA.x;
					((float*)&constraint.anchorA2.y)[j] = rA.y;
					((float*)&constraint.anchorB2.x)[j] = rB.x;
					((float*)&constraint.anchorB2.y)[j] = rB.y;

					((float*)&constraint.baseSeparation2)[j] = mp.separation - (rB - rA).Dot(normal);

					((float*)&constraint.normalImpulse2)[j] = warmStartScale * mp.normalImpulse;
					((float*)&constraint.tangentImpulse2)[j] = warmStartScale * mp.tangentImpulse;
					((float*)&constraint.maxNormalImpulse2)[j] = 0.0f;

					float rnA = rA.Cross(normal);
					float rnB = rB.Cross(normal);
					float kNormal = mA + mB + iA * rnA * rnA + iB * rnB * rnB;
					((float*)&constraint.normalMass2)[j] = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

					float rtA = rA.Cross(tangent);
					float rtB = rB.Cross(tangent);
					float kTangent = mA + mB + iA * rtA * rtA + iB * rtB * rtB;
					((float*)&constraint.tangentMass2)[j] = kTangent > 0.0f ? 1.0f / kTangent : 0.0f;

					// relative velocity for restitution
					Vector2 vrA = vA + rA.CrossInv(wA);
					Vector2 vrB = vB + rB.CrossInv(wB);
					((float*)&constraint.relativeVelocity2)[j] = normal.Dot(vrB - vrA);
				}
				else
				{
					// dummy data that has no effect
					((float*)&constraint.baseSeparation2)[j] = 0.0f;
					((float*)&constraint.normalImpulse2)[j] = 0.0f;
					((float*)&constraint.tangentImpulse2)[j] = 0.0f;
					((float*)&constraint.maxNormalImpulse2)[j] = 0.0f;
					((float*)&constraint.anchorA2.x)[j] = 0.0f;
					((float*)&constraint.anchorA2.y)[j] = 0.0f;
					((float*)&constraint.anchorB2.x)[j] = 0.0f;
					((float*)&constraint.anchorB2.y)[j] = 0.0f;
					((float*)&constraint.normalMass2)[j] = 0.0f;
					((float*)&constraint.tangentMass2)[j] = 0.0f;
					((float*)&constraint.relativeVelocity2)[j] = 0.0f;
				}
			}
			else
			{
				// SIMD remainder
				constraint.indexA[j] = NullIndex;
				constraint.indexB[j] = NullIndex;

				((float*)&constraint.invMassA)[j] = 0.0f;
				((float*)&constraint.invMassB)[j] = 0.0f;
				((float*)&constraint.invIA)[j] = 0.0f;
				((float*)&constraint.invIB)[j] = 0.0f;

				((float*)&constraint.normal.x)[j] = 0.0f;
				((float*)&constraint.normal.y)[j] = 0.0f;
				((float*)&constraint.friction)[j] = 0.0f;
				((float*)&constraint.biasRate)[j] = 0.0f;
				((float*)&constraint.massScale)[j] = 0.0f;
				((float*)&constraint.impulseScale)[j] = 0.0f;

				((float*)&constraint.anchorA1.x)[j] = 0.0f;
				((float*)&constraint.anchorA1.y)[j] = 0.0f;
				((float*)&constraint.anchorB1.x)[j] = 0.0f;
				((float*)&constraint.anchorB1.y)[j] = 0.0f;
				((float*)&constraint.baseSeparation1)[j] = 0.0f;
				((float*)&constraint.normalImpulse1)[j] = 0.0f;
				((float*)&constraint.tangentImpulse1)[j] = 0.0f;
				((float*)&constraint.maxNormalImpulse1)[j] = 0.0f;
				((float*)&constraint.normalMass1)[j] = 0.0f;
				((float*)&constraint.tangentMass1)[j] = 0.0f;

				((float*)&constraint.anchorA2.x)[j] = 0.0f;
				((float*)&constraint.anchorA2.y)[j] = 0.0f;
				((float*)&constraint.anchorB2.x)[j] = 0.0f;
				((float*)&constraint.anchorB2.y)[j] = 0.0f;
				((float*)&constraint.baseSeparation2)[j] = 0.0f;
				((float*)&constraint.normalImpulse2)[j] = 0.0f;
				((float*)&constraint.tangentImpulse2)[j] = 0.0f;
				((float*)&constraint.maxNormalImpulse2)[j] = 0.0f;
				((float*)&constraint.normalMass2)[j] = 0.0f;
				((float*)&constraint.tangentMass2)[j] = 0.0f;

				((float*)&constraint.restitution)[j] = 0.0f;
				((float*)&constraint.relativeVelocity1)[j] = 0.0f;
				((float*)&constraint.relativeVelocity2)[j] = 0.0f;
			}
		}
	}
}

void Physics::WarmStartContactsTask(int startIndex, int endIndex, StepContext& context, int colorIndex)
{
	BodyState* states = context.states;
	ContactConstraintSIMD* constraints = context.graph->colors[colorIndex].simdConstraints;

	for (int i = startIndex; i < endIndex; ++i)
	{
		ContactConstraintSIMD& c = constraints[i];
		SimdBody bA = GatherBodies(states, c.indexA);
		SimdBody bB = GatherBodies(states, c.indexB);

		FloatW tangentX = c.normal.y;
		FloatW tangentY = ZeroFloatW - c.normal.x;

		{
			// fixed anchors
			Vector2W rA = c.anchorA1;
			Vector2W rB = c.anchorB1;

			Vector2W P;
			P.x = (c.normalImpulse1 * c.normal.x) + (c.tangentImpulse1 * tangentX);
			P.y = (c.normalImpulse1 * c.normal.y) + (c.tangentImpulse1 * tangentY);
			bA.w = MulSubW(bA.w, c.invIA, CrossW(rA, P));
			bA.v.x = MulSubW(bA.v.x, c.invMassA, P.x);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, P.y);
			bB.w = MulAddW(bB.w, c.invIB, CrossW(rB, P));
			bB.v.x = MulAddW(bB.v.x, c.invMassB, P.x);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, P.y);
		}

		{
			// fixed anchors
			Vector2W rA = c.anchorA2;
			Vector2W rB = c.anchorB2;

			Vector2W P;
			P.x = (c.normalImpulse2 * c.normal.x) + (c.tangentImpulse2 * tangentX);
			P.y = (c.normalImpulse2 * c.normal.y) + (c.tangentImpulse2 * tangentY);
			bA.w = MulSubW(bA.w, c.invIA, CrossW(rA, P));
			bA.v.x = MulSubW(bA.v.x, c.invMassA, P.x);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, P.y);
			bB.w = MulAddW(bB.w, c.invIB, CrossW(rB, P));
			bB.v.x = MulAddW(bB.v.x, c.invMassB, P.x);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, P.y);
		}

		ScatterBodies(states, c.indexA, bA);
		ScatterBodies(states, c.indexB, bB);
	}
}

void Physics::SolveContactsTask(int startIndex, int endIndex, StepContext& context, int colorIndex, bool useBias)
{
	BodyState* states = context.states;
	ContactConstraintSIMD* constraints = context.graph->colors[colorIndex].simdConstraints;
	FloatW inv_h = SplatW(context.inv_h);
	FloatW minBiasVel = SplatW(-contactPushoutVelocity);

	for (int i = startIndex; i < endIndex; ++i)
	{
		ContactConstraintSIMD& c = constraints[i];

		SimdBody bA = GatherBodies(states, c.indexA);
		SimdBody bB = GatherBodies(states, c.indexB);

		FloatW biasRate, massScale, impulseScale;
		if (useBias)
		{
			biasRate = c.biasRate;
			massScale = c.massScale;
			impulseScale = c.impulseScale;
		}
		else
		{
			biasRate = ZeroFloatW;
			massScale = SplatW(1.0f);
			impulseScale = ZeroFloatW;
		}

		Vector2W dp = { bB.dp.x - bA.dp.x, bB.dp.y - bA.dp.y };

		// point1 non-penetration constraint
		{
			// moving anchors for current separation
			Vector2W rsA = RotateVectorW(bA.dq, c.anchorA1);
			Vector2W rsB = RotateVectorW(bB.dq, c.anchorB1);

			// compute current separation
			// this is subject to round-off error if the anchor is far from the body center of mass
			Vector2W ds = { dp.x + (rsB.x - rsA.x), dp.y + (rsB.y - rsA.y) };
			FloatW s = DotW(c.normal, ds) + c.baseSeparation1;

			// Apply speculative bias if separation is greater than zero, otherwise apply soft constraint bias
			FloatW mask = s > ZeroFloatW;
			FloatW specBias = s * inv_h;
			FloatW softBias = MaxW(biasRate * s, minBiasVel);
			FloatW bias = BlendW(softBias, specBias, mask);

			// fixed anchors for Jacobians
			Vector2W rA = c.anchorA1;
			Vector2W rB = c.anchorB1;

			// Relative velocity at contact
			FloatW dvx = (bB.v.x - bB.w * rB.y) - (bA.v.x - bA.w * rA.y);
			FloatW dvy = (bB.v.y + bB.w * rB.x) - (bA.v.y + bA.w * rA.x);
			FloatW vn = dvx * c.normal.x + dvy * c.normal.y;

			// Compute normal impulse
			FloatW negImpulse = c.normalMass1 * massScale * (vn + bias) + impulseScale * c.normalImpulse1;

			// Clamp the accumulated impulse
			FloatW newImpulse = MaxW(c.normalImpulse1 - negImpulse, ZeroFloatW);
			FloatW impulse = newImpulse - c.normalImpulse1;
			c.normalImpulse1 = newImpulse;
			c.maxNormalImpulse1 = MaxW(c.maxNormalImpulse1, newImpulse);

			// Apply contact impulse
			FloatW Px = impulse * c.normal.x;
			FloatW Py = impulse * c.normal.y;

			bA.v.x = MulSubW(bA.v.x, c.invMassA, Px);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, Py);
			bA.w = MulSubW(bA.w, c.invIA, rA.x * Py - rA.y * Px);

			bB.v.x = MulAddW(bB.v.x, c.invMassB, Px);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, Py);
			bB.w = MulAddW(bB.w, c.invIB, rB.x * Py - rB.y * Px);
		}

		// second point non-penetration constraint
		{
			// moving anchors for current separation
			Vector2W rsA = RotateVectorW(bA.dq, c.anchorA2);
			Vector2W rsB = RotateVectorW(bB.dq, c.anchorB2);

			// compute current separation
			Vector2W ds = { (dp.x + (rsB.x - rsA.x)), (dp.y + (rsB.y - rsA.y)) };
			FloatW s = DotW(c.normal, ds) + c.baseSeparation2;

			FloatW mask = s > ZeroFloatW;
			FloatW specBias = s * inv_h;
			FloatW softBias = MaxW(biasRate * s, minBiasVel);
			FloatW bias = BlendW(softBias, specBias, mask);

			// fixed anchors for Jacobians
			Vector2W rA = c.anchorA2;
			Vector2W rB = c.anchorB2;

			// Relative velocity at contact
			FloatW dvx = (bB.v.x - bB.w * rB.y) - (bA.v.x - bA.w * rA.y);
			FloatW dvy = (bB.v.y + bB.w * rB.x) - (bA.v.y + bA.w * rA.x);
			FloatW vn = dvx * c.normal.x + dvy * c.normal.y;

			// Compute normal impulse
			FloatW negImpulse = c.normalMass2 * massScale * (vn + bias) + impulseScale * c.normalImpulse2;

			// Clamp the accumulated impulse
			FloatW newImpulse = MaxW(c.normalImpulse2 - negImpulse, ZeroFloatW);
			FloatW impulse = newImpulse - c.normalImpulse2;
			c.normalImpulse2 = newImpulse;
			c.maxNormalImpulse2 = MaxW(c.maxNormalImpulse2, newImpulse);

			// Apply contact impulse
			FloatW Px = impulse * c.normal.x;
			FloatW Py = impulse * c.normal.y;

			bA.v.x = MulSubW(bA.v.x, c.invMassA, Px);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, Py);
			bA.w = MulSubW(bA.w, c.invIA, rA.x * Py - rA.y * Px);

			bB.v.x = MulAddW(bB.v.x, c.invMassB, Px);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, Py);
			bB.w = MulAddW(bB.w, c.invIB, rB.x * Py - rB.y * Px);
		}

		FloatW tangentX = c.normal.y;
		FloatW tangentY = ZeroFloatW - c.normal.x;

		// point 1 friction constraint
		{
			// fixed anchors for Jacobians
			Vector2W rA = c.anchorA1;
			Vector2W rB = c.anchorB1;

			// Relative velocity at contact
			FloatW dvx = (bB.v.x - bB.w * rB.y) - (bA.v.x - bA.w * rA.y);
			FloatW dvy = (bB.v.y + bB.w * rB.x) - (bA.v.y + bA.w * rA.x);
			FloatW vt = dvx * tangentX + dvy * tangentY;

			// Compute tangent force
			FloatW negImpulse = c.tangentMass1 * vt;

			// Clamp the accumulated force
			FloatW maxFriction = c.friction * c.normalImpulse1;
			FloatW newImpulse = c.tangentImpulse1 - negImpulse;
			newImpulse = MaxW(ZeroFloatW - maxFriction, MinW(newImpulse, maxFriction));
			FloatW impulse = newImpulse - c.tangentImpulse1;
			c.tangentImpulse1 = newImpulse;

			// Apply contact impulse
			FloatW Px = impulse * tangentX;
			FloatW Py = impulse * tangentY;

			bA.v.x = MulSubW(bA.v.x, c.invMassA, Px);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, Py);
			bA.w = MulSubW(bA.w, c.invIA, rA.x * Py - rA.y * Px);

			bB.v.x = MulAddW(bB.v.x, c.invMassB, Px);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, Py);
			bB.w = MulAddW(bB.w, c.invIB, rB.x * Py - rB.y * Px);
		}

		// second point friction constraint
		{
			// fixed anchors for Jacobians
			Vector2W rA = c.anchorA2;
			Vector2W rB = c.anchorB2;

			// Relative velocity at contact
			FloatW dvx = (bB.v.x - bB.w * rB.y) - (bA.v.x - bA.w * rA.y);
			FloatW dvy = (bB.v.y + bB.w * rB.x) - (bA.v.y + bA.w * rA.x);
			FloatW vt = dvx * tangentX + dvy * tangentY;

			// Compute tangent force
			FloatW negImpulse = c.tangentMass2 * vt;

			// Clamp the accumulated force
			FloatW maxFriction = c.friction * c.normalImpulse2;
			FloatW newImpulse = c.tangentImpulse2 - negImpulse;
			newImpulse = MaxW(ZeroFloatW - maxFriction, MinW(newImpulse, maxFriction));
			FloatW impulse = newImpulse - c.tangentImpulse2;
			c.tangentImpulse2 = newImpulse;

			// Apply contact impulse
			FloatW Px = impulse * tangentX;
			FloatW Py = impulse * tangentY;

			bA.v.x = MulSubW(bA.v.x, c.invMassA, Px);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, Py);
			bA.w = MulSubW(bA.w, c.invIA, rA.x * Py - rA.y * Px);

			bB.v.x = MulAddW(bB.v.x, c.invMassB, Px);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, Py);
			bB.w = MulAddW(bB.w, c.invIB, rB.x * Py - rB.y * Px);
		}

		ScatterBodies(states, c.indexA, bA);
		ScatterBodies(states, c.indexB, bB);
	}
}

void Physics::CreateContact(Shape& shapeA, Shape& shapeB)
{
	ShapeType type1 = shapeA.type;
	ShapeType type2 = shapeB.type;

	if (contactRegister[type1][type2].fcn == nullptr) { return; }

	if (contactRegister[type1][type2].primary == false)
	{
		CreateContact(shapeB, shapeA);
		return;
	}

	RigidBody2D& bodyA = rigidBodies[shapeA.bodyId];
	RigidBody2D& bodyB = rigidBodies[shapeB.bodyId];

	I32 setIndex;
	if (bodyA.setIndex == SET_TYPE_AWAKE || bodyB.setIndex == SET_TYPE_AWAKE)
	{
		setIndex = SET_TYPE_AWAKE;
	}
	else
	{
		// sleeping and non-touching contacts live in the disabled set
		// later if this set is found to be touching then the sleeping
		// islands will be linked and the contact moved to the merged island
		setIndex = SET_TYPE_DISABLED;
	}

	SolverSet& set = solverSets[setIndex];

	// Create contact key and contact
	I32 contactId = contactFreelist.GetFree();
	if (contactId == contacts.Size())
	{
		contacts.Push({});
	}

	I32 shapeIdA = shapeA.id;
	I32 shapeIdB = shapeB.id;

	Contact& contact = contacts[contactId];
	contact.contactId = contactId;
	contact.setIndex = setIndex;
	contact.colorIndex = NullIndex;
	contact.localIndex = (I32)set.contactSims.Size();
	contact.islandId = NullIndex;
	contact.islandPrev = NullIndex;
	contact.islandNext = NullIndex;
	contact.shapeIdA = shapeIdA;
	contact.shapeIdB = shapeIdB;
	contact.isMarked = false;
	contact.flags = 0;

	if (shapeA.isSensor || shapeB.isSensor)
	{
		contact.flags |= CONTACT_FLAG_SENSOR;
	}

	if (shapeA.enableSensorEvents || shapeB.enableSensorEvents)
	{
		contact.flags |= CONTACT_FLAG_ENABLE_SENSOR_EVENTS;
	}

	if (shapeA.enableContactEvents || shapeB.enableContactEvents)
	{
		contact.flags |= CONTACT_FLAG_ENABLE_CONTACT_EVENTS;
	}

	// Connect to body A
	{
		contact.edges[0].bodyId = shapeA.bodyId;
		contact.edges[0].prevKey = NullIndex;
		contact.edges[0].nextKey = bodyA.headContactKey;

		I32 keyA = (contactId << 1) | 0;
		I32 headContactKey = bodyA.headContactKey;
		if (headContactKey != NullIndex)
		{
			Contact& headContact = contacts[headContactKey >> 1];
			headContact.edges[headContactKey & 1].prevKey = keyA;
		}
		bodyA.headContactKey = keyA;
		bodyA.contactCount += 1;
	}

	// Connect to body B
	{
		contact.edges[1].bodyId = shapeB.bodyId;
		contact.edges[1].prevKey = NullIndex;
		contact.edges[1].nextKey = bodyB.headContactKey;

		I32 keyB = (contactId << 1) | 1;
		I32 headContactKey = bodyB.headContactKey;
		if (bodyB.headContactKey != NullIndex)
		{
			Contact& headContact = contacts[headContactKey >> 1];
			headContact.edges[headContactKey & 1].prevKey = keyB;
		}
		bodyB.headContactKey = keyB;
		bodyB.contactCount += 1;
	}

	// Add to pair set for fast lookup
	U64 pairKey = SHAPE_PAIR_KEY(shapeIdA, shapeIdB);
	Broadphase::pairSet.Insert(pairKey);

	// Contacts are created as non-touching. Later if they are found to be touching
	// they will link islands and be moved into the constraint graph.
	ContactSim& contactSim = set.contactSims.Push({});
	contactSim.contactId = contactId;

	contactSim.bodySimIndexA = NullIndex;
	contactSim.bodySimIndexB = NullIndex;
	contactSim.invMassA = 0.0f;
	contactSim.invIA = 0.0f;
	contactSim.invMassB = 0.0f;
	contactSim.invIB = 0.0f;
	contactSim.shapeIdA = shapeIdA;
	contactSim.shapeIdB = shapeIdB;
	contactSim.cache = {};
	contactSim.manifold = {};
	contactSim.friction = MixFriction(shapeA.friction, shapeB.friction);
	contactSim.restitution = MixRestitution(shapeA.restitution, shapeB.restitution);
	contactSim.tangentSpeed = 0.0f;
	contactSim.simFlags = 0;

	if (shapeA.enablePreSolveEvents || shapeB.enablePreSolveEvents)
	{
		contactSim.simFlags |= CONTACT_SIM_FLAG_ENABLE_PRESOLVE_EVENTS;
	}
}

void Physics::DestroyContact(Contact& contact, bool wakeBodies)
{
	// Remove pair from set
	U64 pairKey = SHAPE_PAIR_KEY(contact.shapeIdA, contact.shapeIdB);
	Broadphase::pairSet.Remove(pairKey);

	ContactEdge& edgeA = contact.edges[0];
	ContactEdge& edgeB = contact.edges[1];

	int bodyIdA = edgeA.bodyId;
	int bodyIdB = edgeB.bodyId;
	RigidBody2D& bodyA = rigidBodies[bodyIdA];
	RigidBody2D& bodyB = rigidBodies[bodyIdB];

	// if (contactListener && contact->IsTouching())
	//{
	//	contactListener->EndContact(contact);
	// }

	// Remove from body A
	if (edgeA.prevKey != NullIndex)
	{
		Contact& prevContact = contacts[edgeA.prevKey >> 1];
		ContactEdge& prevEdge = prevContact.edges[edgeA.prevKey & 1];
		prevEdge.nextKey = edgeA.nextKey;
	}

	if (edgeA.nextKey != NullIndex)
	{
		Contact& nextContact = contacts[edgeA.nextKey >> 1];
		ContactEdge& nextEdge = nextContact.edges[edgeA.nextKey & 1];
		nextEdge.prevKey = edgeA.prevKey;
	}

	int contactId = contact.contactId;

	int edgeKeyA = (contactId << 1) | 0;
	if (bodyA.headContactKey == edgeKeyA)
	{
		bodyA.headContactKey = edgeA.nextKey;
	}

	bodyA.contactCount -= 1;

	// Remove from body B
	if (edgeB.prevKey != NullIndex)
	{
		Contact& prevContact = contacts[edgeB.prevKey >> 1];
		ContactEdge& prevEdge = prevContact.edges[edgeB.prevKey & 1];
		prevEdge.nextKey = edgeB.nextKey;
	}

	if (edgeB.nextKey != NullIndex)
	{
		Contact& nextContact = contacts[edgeB.nextKey >> 1];
		ContactEdge& nextEdge = nextContact.edges[edgeB.nextKey & 1];
		nextEdge.prevKey = edgeB.prevKey;
	}

	int edgeKeyB = (contactId << 1) | 1;
	if (bodyB.headContactKey == edgeKeyB)
	{
		bodyB.headContactKey = edgeB.nextKey;
	}

	bodyB.contactCount -= 1;

	// Remove contact from the array that owns it
	if (contact.islandId != NullIndex)
	{
		UnlinkContact(contact);
	}

	if (contact.colorIndex != NullIndex)
	{
		constraintGraph.RemoveContact(bodyIdA, bodyIdB, contact.colorIndex, contact.localIndex);
	}
	else
	{
		// contact is non-touching or is sleeping or is a sensor
		SolverSet& set = solverSets[contact.setIndex];
		int movedIndex = set.contactSims.RemoveSwap(contact.localIndex);
		if (movedIndex != NullIndex)
		{
			ContactSim& movedContactSim = set.contactSims[contact.localIndex];
			Contact& movedContact = contacts[movedContactSim.contactId];
			movedContact.localIndex = contact.localIndex;
		}
	}

	contact.contactId = NullIndex;
	contact.setIndex = NullIndex;
	contact.colorIndex = NullIndex;
	contact.localIndex = NullIndex;

	contactFreelist.Release(contactId);

	if (wakeBodies)
	{
		WakeBody(bodyA);
		WakeBody(bodyB);
	}
}

bool Physics::UpdateContact(ContactSim& contactSim, Shape& shapeA, const Transform2D& transformA, const Vector2& centerOffsetA,
	Shape& shapeB, const Transform2D& transformB, const Vector2& centerOffsetB)
{
	bool touching;

	// Is this contact a sensor?
	if (shapeA.isSensor || shapeB.isSensor)
	{
		// Sensors don't generate manifolds or hit events
		touching = TestShapeOverlap(shapeA, transformA, shapeB, transformB, contactSim.cache);
	}
	else
	{
		Manifold oldManifold = contactSim.manifold;

		// Compute TOI
		ManifoldFn* fcn = contactRegister[shapeA.type][shapeB.type].fcn;

		contactSim.manifold = fcn(shapeA, transformA, shapeB, transformB, contactSim.cache);

		int pointCount = contactSim.manifold.pointCount;
		touching = pointCount > 0;

		//TODO: preSolve function?
		//if (touching && world->preSolveFcn && (contactSim.simFlags & CONTACT_SIM_FLAG_ENABLE_PRESOLVE_EVENTS) != 0)
		//{
		//	I32 shapeIdA = shapeA.id + 1;
		//	I32 shapeIdB = shapeB.id + 1;
		//
		//	// this call assumes thread safety
		//	touching = world->preSolveFcn(shapeIdA, shapeIdB, &contactSim.manifold, world->preSolveContext);
		//	if (touching == false)
		//	{
		//		// disable contact
		//		contactSim.manifold.pointCount = 0;
		//	}
		//}

		if (touching && (shapeA.enableHitEvents || shapeB.enableHitEvents))
		{
			contactSim.simFlags |= CONTACT_SIM_FLAG_ENABLE_HIT_EVENT;
		}
		else
		{
			contactSim.simFlags &= ~CONTACT_SIM_FLAG_ENABLE_HIT_EVENT;
		}

		// Match old contact ids to new contact ids and copy the
		// stored impulses to warm start the solver.
		for (int i = 0; i < pointCount; ++i)
		{
			ManifoldPoint& mp2 = contactSim.manifold.points[i];

			// shift anchors to be center of mass relative
			mp2.anchorA = mp2.anchorA - centerOffsetA;
			mp2.anchorB = mp2.anchorB - centerOffsetB;

			mp2.normalImpulse = 0.0f;
			mp2.tangentImpulse = 0.0f;
			mp2.maxNormalImpulse = 0.0f;
			mp2.normalVelocity = 0.0f;
			mp2.persisted = false;

			U16 id2 = mp2.id;

			for (U32 j = 0; j < oldManifold.pointCount; ++j)
			{
				ManifoldPoint& mp1 = oldManifold.points[j];

				if (mp1.id == id2)
				{
					mp2.normalImpulse = mp1.normalImpulse;
					mp2.tangentImpulse = mp1.tangentImpulse;
					mp2.persisted = true;
					break;
				}
			}
		}
	}

	if (touching)
	{
		contactSim.simFlags |= CONTACT_SIM_FLAG_TOUCHING;
	}
	else
	{
		contactSim.simFlags &= ~CONTACT_SIM_FLAG_TOUCHING;
	}

	return touching;
}

void Physics::LinkContact(Contact& contact)
{
	int bodyIdA = contact.edges[0].bodyId;
	int bodyIdB = contact.edges[1].bodyId;

	RigidBody2D& bodyA = rigidBodies[bodyIdA];
	RigidBody2D& bodyB = rigidBodies[bodyIdB];

	// Wake bodyB if bodyA is awake and bodyB is sleeping
	if (bodyA.setIndex == SET_TYPE_AWAKE && bodyB.setIndex >= SET_TYPE_FIRST_SLEEPING)
	{
		WakeSolverSet(bodyB.setIndex);
	}

	// Wake bodyA if bodyB is awake and bodyA is sleeping
	if (bodyB.setIndex == SET_TYPE_AWAKE && bodyA.setIndex >= SET_TYPE_FIRST_SLEEPING)
	{
		WakeSolverSet(bodyA.setIndex);
	}

	int islandIdA = bodyA.islandId;
	int islandIdB = bodyB.islandId;

	if (islandIdA == islandIdB)
	{
		// Contact in same island
		AddContactToIsland(islandIdA, contact);
		return;
	}

	// Union-find root of islandA
	Island* islandA = nullptr;
	if (islandIdA != NullIndex)
	{
		islandA = &islands[islandIdA];
		int parentId = islandA->parentIsland;
		while (parentId != NullIndex)
		{
			Island* parent = &islands[parentId];
			if (parent->parentIsland != NullIndex)
			{
				// path compression
				islandA->parentIsland = parent->parentIsland;
			}

			islandA = parent;
			islandIdA = parentId;
			parentId = islandA->parentIsland;
		}
	}

	// Union-find root of islandB
	Island* islandB = nullptr;
	if (islandIdB != NullIndex)
	{
		islandB = &islands[islandIdB];
		int parentId = islandB->parentIsland;
		while (islandB->parentIsland != NullIndex)
		{
			Island* parent = &islands[parentId];
			if (parent->parentIsland != NullIndex)
			{
				// path compression
				islandB->parentIsland = parent->parentIsland;
			}

			islandB = parent;
			islandIdB = parentId;
			parentId = islandB->parentIsland;
		}
	}

	// Union-Find link island roots
	if (islandA != islandB && islandA != nullptr && islandB != nullptr)
	{
		islandB->parentIsland = islandIdA;
	}

	if (islandA != nullptr) { AddContactToIsland(islandIdA, contact); }
	else { AddContactToIsland(islandIdB, contact); }
}

void Physics::UnlinkContact(Contact& contact)
{
	// remove from island
	int islandId = contact.islandId;
	Island& island = islands[islandId];

	if (contact.islandPrev != NullIndex)
	{
		Contact& prevContact = contacts[contact.islandPrev];
		prevContact.islandNext = contact.islandNext;
	}

	if (contact.islandNext != NullIndex)
	{
		Contact& nextContact = contacts[contact.islandNext];
		nextContact.islandPrev = contact.islandPrev;
	}

	if (island.headContact == contact.contactId)
	{
		island.headContact = contact.islandNext;
	}

	if (island.tailContact == contact.contactId)
	{
		island.tailContact = contact.islandPrev;
	}

	island.contactCount -= 1;
	island.constraintRemoveCount += 1;

	contact.islandId = NullIndex;
	contact.islandPrev = NullIndex;
	contact.islandNext = NullIndex;
}

void Physics::DestroyJointInternal(Joint& joint, bool wakeBodies)
{
	int jointId = joint.jointId;

	JointEdge& edgeA = joint.edges[0];
	JointEdge& edgeB = joint.edges[1];

	int idA = edgeA.bodyId;
	int idB = edgeB.bodyId;
	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	// Remove from body A
	if (edgeA.prevKey != NullIndex)
	{
		Joint& prevJoint = joints[edgeA.prevKey >> 1];
		JointEdge& prevEdge = prevJoint.edges[edgeA.prevKey & 1];
		prevEdge.nextKey = edgeA.nextKey;
	}

	if (edgeA.nextKey != NullIndex)
	{
		Joint& nextJoint = joints[edgeA.nextKey >> 1];
		JointEdge& nextEdge = nextJoint.edges[edgeA.nextKey & 1];
		nextEdge.prevKey = edgeA.prevKey;
	}

	int edgeKeyA = (jointId << 1) | 0;
	if (bodyA.headJointKey == edgeKeyA)
	{
		bodyA.headJointKey = edgeA.nextKey;
	}

	bodyA.jointCount -= 1;

	// Remove from body B
	if (edgeB.prevKey != NullIndex)
	{
		Joint& prevJoint = joints[edgeB.prevKey >> 1];
		JointEdge& prevEdge = prevJoint.edges[edgeB.prevKey & 1];
		prevEdge.nextKey = edgeB.nextKey;
	}

	if (edgeB.nextKey != NullIndex)
	{
		Joint& nextJoint = joints[edgeB.nextKey >> 1];
		JointEdge& nextEdge = nextJoint.edges[edgeB.nextKey & 1];
		nextEdge.prevKey = edgeB.prevKey;
	}

	int edgeKeyB = (jointId << 1) | 1;
	if (bodyB.headJointKey == edgeKeyB)
	{
		bodyB.headJointKey = edgeB.nextKey;
	}

	bodyB.jointCount -= 1;

	if (joint.islandId != NullIndex)
	{
		UnlinkJoint(joint);
	}

	// Remove joint from solver set that owns it
	int setIndex = joint.setIndex;
	int localIndex = joint.localIndex;

	if (setIndex == SET_TYPE_AWAKE)
	{
		RemoveJointFromGraph(joint.edges[0].bodyId, joint.edges[1].bodyId, joint.colorIndex, localIndex);
	}
	else
	{
		SolverSet& set = solverSets[setIndex];
		int movedIndex = set.jointSims.RemoveSwap(localIndex);
		if (movedIndex != NullIndex)
		{
			// Fix moved joint
			JointSim& movedJointSim = set.jointSims[localIndex];
			int movedId = movedJointSim.jointId;
			Joint& movedJoint = joints[movedId];
			movedJoint.localIndex = localIndex;
		}
	}

	// Free joint and id (preserve joint revision)
	joint.setIndex = NullIndex;
	joint.localIndex = NullIndex;
	joint.colorIndex = NullIndex;
	joint.jointId = NullIndex;
	jointFreelist.Release(jointId);

	if (wakeBodies)
	{
		WakeBody(bodyA);
		WakeBody(bodyB);
	}
}

void Physics::UnlinkJoint(Joint& joint)
{
	// remove from island
	int islandId = joint.islandId;
	Island& island = islands[islandId];

	if (joint.islandPrev != NullIndex)
	{
		Joint& prevJoint = joints[joint.islandPrev];
		prevJoint.islandNext = joint.islandNext;
	}

	if (joint.islandNext != NullIndex)
	{
		Joint& nextJoint = joints[joint.islandNext];
		nextJoint.islandPrev = joint.islandPrev;
	}

	if (island.headJoint == joint.jointId)
	{
		island.headJoint = joint.islandNext;
	}

	if (island.tailJoint == joint.jointId)
	{
		island.tailJoint = joint.islandPrev;
	}

	island.jointCount -= 1;
	island.constraintRemoveCount += 1;

	joint.islandId = NullIndex;
	joint.islandPrev = NullIndex;
	joint.islandNext = NullIndex;
}

void Physics::RemoveJointFromGraph(int bodyIdA, int bodyIdB, int colorIndex, int localIndex)
{
	GraphColor& color = constraintGraph.colors[colorIndex];

	if (colorIndex != OverflowIndex)
	{
		// May clear static bodies, no effect
		color.bodySet.ClearBit(bodyIdA);
		color.bodySet.ClearBit(bodyIdB);
	}

	int movedIndex = color.jointSims.RemoveSwap(localIndex);
	if (movedIndex != NullIndex)
	{
		// Fix moved joint
		JointSim& movedJointSim = color.jointSims[localIndex];
		int movedId = movedJointSim.jointId;
		Joint& movedJoint = joints[movedId];
		movedJoint.localIndex = localIndex;
	}
}

void Physics::PrepareOverflowJoints(StepContext& context)
{
	ConstraintGraph* graph = context.graph;
	JointSim* joints = graph->colors[OverflowIndex].jointSims.Data();
	int jointCount = (I32)graph->colors[OverflowIndex].jointSims.Size();

	for (int i = 0; i < jointCount; ++i)
	{
		JointSim& joint = joints[i];
		PrepareJoint(joint, context);
	}
}

void Physics::PrepareJointsTask(int startIndex, int endIndex, StepContext& context)
{
	JointSim** joints = context.joints;

	for (int i = startIndex; i < endIndex; ++i)
	{
		JointSim* joint = joints[i];
		PrepareJoint(*joint, context);
	}
}

void Physics::PrepareJoint(JointSim& joint, StepContext& context)
{
	switch (joint.type)
	{
	case JOINT_TYPE_DISTANCE: PrepareDistanceJoint(joint, context); break;
	case JOINT_TYPE_MOTOR: PrepareMotorJoint(joint, context); break;
	case JOINT_TYPE_MOUSE: PrepareMouseJoint(joint, context); break;
	case JOINT_TYPE_PRISMATIC: PreparePrismaticJoint(joint, context); break;
	case JOINT_TYPE_REVOLUTE: PrepareRevoluteJoint(joint, context); break;
	case JOINT_TYPE_WELD: PrepareWeldJoint(joint, context); break;
	case JOINT_TYPE_WHEEL: PrepareWheelJoint(joint, context); break;
	default: break;
	}
}

void Physics::PrepareDistanceJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idA = base.bodyIdA;
	int idB = base.bodyIdB;

	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setA = solverSets[bodyA.setIndex];
	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexA = bodyA.localIndex;
	int localIndexB = bodyB.localIndex;

	BodySim& bodySimA = setA.bodySims[localIndexA];
	BodySim& bodySimB = setB.bodySims[localIndexB];

	float mA = bodySimA.invMass;
	float iA = bodySimA.invInertia;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	base.invMassA = mA;
	base.invMassB = mB;
	base.invIA = iA;
	base.invIB = iB;

	DistanceJoint& joint = base.distanceJoint;

	joint.indexA = bodyA.setIndex == SET_TYPE_AWAKE ? localIndexA : NullIndex;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;

	// initial anchors in world space
	joint.anchorA = (base.localOriginAnchorA - bodySimA.localCenter) * bodySimA.transform.rotation;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * bodySimB.transform.rotation;
	joint.deltaCenter = bodySimB.center - bodySimA.center;

	Vector2 rA = joint.anchorA;
	Vector2 rB = joint.anchorB;
	Vector2 separation = (rB - rA) + joint.deltaCenter;
	Vector2 axis = separation.Normalized();

	// compute effective mass
	float crA = rA.Cross(axis);
	float crB = rB.Cross(axis);
	float k = mA + mB + iA * crA * crA + iB * crB * crB;
	joint.axialMass = k > 0.0f ? 1.0f / k : 0.0f;

	joint.distanceSoftness.Create(joint.hertz, joint.dampingRatio, context.h);

	if (context.enableWarmStarting == false)
	{
		joint.impulse = 0.0f;
		joint.lowerImpulse = 0.0f;
		joint.upperImpulse = 0.0f;
		joint.motorImpulse = 0.0f;
	}
}

void Physics::PrepareMotorJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idA = base.bodyIdA;
	int idB = base.bodyIdB;

	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setA = solverSets[bodyA.setIndex];
	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexA = bodyA.localIndex;
	int localIndexB = bodyB.localIndex;

	BodySim& bodySimA = setA.bodySims[localIndexA];
	BodySim& bodySimB = setB.bodySims[localIndexB];

	float mA = bodySimA.invMass;
	float iA = bodySimA.invInertia;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	base.invMassA = mA;
	base.invMassB = mB;
	base.invIA = iA;
	base.invIB = iB;

	MotorJoint& joint = base.motorJoint;
	joint.indexA = bodyA.setIndex == SET_TYPE_AWAKE ? localIndexA : NullIndex;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;

	joint.anchorA = (base.localOriginAnchorA - bodySimA.localCenter) * bodySimA.transform.rotation;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * bodySimB.transform.rotation;
	joint.deltaCenter = (bodySimB.center - bodySimA.center) - joint.linearOffset;
	joint.deltaAngle = bodySimB.transform.rotation.RelativeAngle(bodySimA.transform.rotation) - joint.angularOffset;
	joint.deltaAngle = Math::UnwindAngle(joint.deltaAngle);

	Vector2 rA = joint.anchorA;
	Vector2 rB = joint.anchorB;

	Matrix2 K;
	K.a.x = mA + mB + rA.y * rA.y * iA + rB.y * rB.y * iB;
	K.a.y = -rA.y * rA.x * iA - rB.y * rB.x * iB;
	K.b.x = K.a.y;
	K.b.y = mA + mB + rA.x * rA.x * iA + rB.x * rB.x * iB;
	joint.linearMass = K.Inverse();

	float ka = iA + iB;
	joint.angularMass = ka > 0.0f ? 1.0f / ka : 0.0f;

	if (context.enableWarmStarting == false)
	{
		joint.linearImpulse = Vector2Zero;
		joint.angularImpulse = 0.0f;
	}
}

void Physics::PrepareMouseJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idB = base.bodyIdB;

	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexB = bodyB.localIndex;
	BodySim& bodySimB = setB.bodySims[localIndexB];

	base.invMassB = bodySimB.invMass;
	base.invIB = bodySimB.invInertia;

	MouseJoint& joint = base.mouseJoint;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * bodySimB.transform.rotation;

	joint.linearSoftness.Create(joint.hertz, joint.dampingRatio, context.h);

	float angularHertz = 0.5f;
	float angularDampingRatio = 0.1f;
	joint.angularSoftness.Create(angularHertz, angularDampingRatio, context.h);

	Vector2 rB = joint.anchorB;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	// K = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
	//   = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
	//     [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
	Matrix2 K;
	K.a.x = mB + iB * rB.y * rB.y;
	K.a.y = -iB * rB.x * rB.y;
	K.b.x = K.a.y;
	K.b.y = mB + iB * rB.x * rB.x;

	joint.linearMass = K.Inverse();
	joint.deltaCenter = bodySimB.center - joint.targetA;

	if (context.enableWarmStarting == false)
	{
		joint.linearImpulse = Vector2Zero;
		joint.angularImpulse = 0.0f;
	}
}

void Physics::PreparePrismaticJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idA = base.bodyIdA;
	int idB = base.bodyIdB;

	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setA = solverSets[bodyA.setIndex];
	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexA = bodyA.localIndex;
	int localIndexB = bodyB.localIndex;

	BodySim& bodySimA = setA.bodySims[localIndexA];
	BodySim& bodySimB = setB.bodySims[localIndexB];

	float mA = bodySimA.invMass;
	float iA = bodySimA.invInertia;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	base.invMassA = mA;
	base.invMassB = mB;
	base.invIA = iA;
	base.invIB = iB;

	PrismaticJoint& joint = base.prismaticJoint;
	joint.indexA = bodyA.setIndex == SET_TYPE_AWAKE ? localIndexA : NullIndex;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;

	Quaternion2 qA = bodySimA.transform.rotation;
	Quaternion2 qB = bodySimB.transform.rotation;

	joint.anchorA = (base.localOriginAnchorA - bodySimA.localCenter) * qA;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * qB;
	joint.axisA = joint.localAxisA * qA;
	joint.deltaCenter = bodySimB.center - bodySimA.center;
	joint.deltaAngle = qB.RelativeAngle(qA) - joint.referenceAngle;

	Vector2 rA = joint.anchorA;
	Vector2 rB = joint.anchorB;

	Vector2 d = joint.deltaCenter + (rB - rA);
	float a1 = (d + rA).Cross(joint.axisA);
	float a2 = rB.Cross(joint.axisA);

	// effective masses
	float k = mA + mB + iA * a1 * a1 + iB * a2 * a2;
	joint.axialMass = k > 0.0f ? 1.0f / k : 0.0f;

	joint.springSoftness.Create(joint.hertz, joint.dampingRatio, context.h);

	if (context.enableWarmStarting == false)
	{
		joint.impulse = Vector2Zero;
		joint.springImpulse = 0.0f;
		joint.motorImpulse = 0.0f;
		joint.lowerImpulse = 0.0f;
		joint.upperImpulse = 0.0f;
	}
}

void Physics::PrepareRevoluteJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idA = base.bodyIdA;
	int idB = base.bodyIdB;

	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setA = solverSets[bodyA.setIndex];
	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexA = bodyA.localIndex;
	int localIndexB = bodyB.localIndex;

	BodySim& bodySimA = setA.bodySims[localIndexA];
	BodySim& bodySimB = setB.bodySims[localIndexB];

	float mA = bodySimA.invMass;
	float iA = bodySimA.invInertia;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	base.invMassA = mA;
	base.invMassB = mB;
	base.invIA = iA;
	base.invIB = iB;

	RevoluteJoint& joint = base.revoluteJoint;

	joint.indexA = bodyA.setIndex == SET_TYPE_AWAKE ? localIndexA : NullIndex;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;

	// initial anchors in world space
	joint.anchorA = (base.localOriginAnchorA - bodySimA.localCenter) * bodySimA.transform.rotation;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * bodySimB.transform.rotation;
	joint.deltaCenter = bodySimB.center - bodySimA.center;
	joint.deltaAngle = bodySimB.transform.rotation.RelativeAngle(bodySimA.transform.rotation) - joint.referenceAngle;
	joint.deltaAngle = Math::UnwindAngle(joint.deltaAngle);

	float k = iA + iB;
	joint.axialMass = k > 0.0f ? 1.0f / k : 0.0f;

	joint.springSoftness.Create(joint.hertz, joint.dampingRatio, context.h);

	if (context.enableWarmStarting == false)
	{
		joint.linearImpulse = Vector2Zero;
		joint.springImpulse = 0.0f;
		joint.motorImpulse = 0.0f;
		joint.lowerImpulse = 0.0f;
		joint.upperImpulse = 0.0f;
	}
}

void Physics::PrepareWeldJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idA = base.bodyIdA;
	int idB = base.bodyIdB;

	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setA = solverSets[bodyA.setIndex];
	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexA = bodyA.localIndex;
	int localIndexB = bodyB.localIndex;

	BodySim& bodySimA = setA.bodySims[localIndexA];
	BodySim& bodySimB = setB.bodySims[localIndexB];

	float mA = bodySimA.invMass;
	float iA = bodySimA.invInertia;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	base.invMassA = mA;
	base.invMassB = mB;
	base.invIA = iA;
	base.invIB = iB;

	WeldJoint& joint = base.weldJoint;
	joint.indexA = bodyA.setIndex == SET_TYPE_AWAKE ? localIndexA : NullIndex;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;

	Quaternion2 qA = bodySimA.transform.rotation;
	Quaternion2 qB = bodySimB.transform.rotation;

	joint.anchorA = (base.localOriginAnchorA - bodySimA.localCenter) * qA;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * qB;
	joint.deltaCenter = bodySimB.center - bodySimA.center;
	joint.deltaAngle = qB.RelativeAngle(qA) - joint.referenceAngle;

	float ka = iA + iB;
	joint.axialMass = ka > 0.0f ? 1.0f / ka : 0.0f;

	const float h = context.dt;

	if (joint.linearHertz == 0.0f)
	{
		joint.linearSoftness = context.jointSoftness;
	}
	else
	{
		joint.linearSoftness.Create(joint.linearHertz, joint.linearDampingRatio, context.h);
	}

	if (joint.angularHertz == 0.0f)
	{
		joint.angularSoftness = context.jointSoftness;
	}
	else
	{
		joint.angularSoftness.Create(joint.angularHertz, joint.angularDampingRatio, context.h);
	}

	if (context.enableWarmStarting == false)
	{
		joint.linearImpulse = Vector2Zero;
		joint.angularImpulse = 0.0f;
	}
}

void Physics::PrepareWheelJoint(JointSim& base, StepContext& context)
{
	// chase body id to the solver set where the body lives
	int idA = base.bodyIdA;
	int idB = base.bodyIdB;

	RigidBody2D& bodyA = rigidBodies[idA];
	RigidBody2D& bodyB = rigidBodies[idB];

	SolverSet& setA = solverSets[bodyA.setIndex];
	SolverSet& setB = solverSets[bodyB.setIndex];

	int localIndexA = bodyA.localIndex;
	int localIndexB = bodyB.localIndex;

	BodySim& bodySimA = setA.bodySims[localIndexA];
	BodySim& bodySimB = setB.bodySims[localIndexB];

	float mA = bodySimA.invMass;
	float iA = bodySimA.invInertia;
	float mB = bodySimB.invMass;
	float iB = bodySimB.invInertia;

	base.invMassA = mA;
	base.invMassB = mB;
	base.invIA = iA;
	base.invIB = iB;

	WheelJoint& joint = base.wheelJoint;

	joint.indexA = bodyA.setIndex == SET_TYPE_AWAKE ? localIndexA : NullIndex;
	joint.indexB = bodyB.setIndex == SET_TYPE_AWAKE ? localIndexB : NullIndex;

	Quaternion2 qA = bodySimA.transform.rotation;
	Quaternion2 qB = bodySimB.transform.rotation;

	joint.anchorA = (base.localOriginAnchorA - bodySimA.localCenter) * qA;
	joint.anchorB = (base.localOriginAnchorB - bodySimB.localCenter) * qB;
	joint.axisA = joint.localAxisA * qA;
	joint.deltaCenter = bodySimB.center - bodySimA.center;

	Vector2 rA = joint.anchorA;
	Vector2 rB = joint.anchorB;

	Vector2 d = joint.deltaCenter + (rB - rA);
	Vector2 axisA = joint.axisA;
	Vector2 perpA = axisA.PerpendicularLeft();

	// perpendicular constraint (keep wheel on line)
	float s1 = (d + rA).Cross(perpA);
	float s2 = rB.Cross(perpA);

	float kp = mA + mB + iA * s1 * s1 + iB * s2 * s2;
	joint.perpMass = kp > 0.0f ? 1.0f / kp : 0.0f;

	// spring constraint
	float a1 = (d + rA).Cross(axisA);
	float a2 = rB.Cross(axisA);

	float ka = mA + mB + iA * a1 * a1 + iB * a2 * a2;
	joint.axialMass = ka > 0.0f ? 1.0f / ka : 0.0f;

	joint.springSoftness.Create(joint.hertz, joint.dampingRatio, context.h);

	float km = iA + iB;
	joint.motorMass = km > 0.0f ? 1.0f / km : 0.0f;

	if (context.enableWarmStarting == false)
	{
		joint.perpImpulse = 0.0f;
		joint.springImpulse = 0.0f;
		joint.motorImpulse = 0.0f;
		joint.lowerImpulse = 0.0f;
		joint.upperImpulse = 0.0f;
	}
}

void Physics::WarmStartOverflowJoints(StepContext& context)
{
	ConstraintGraph* graph = context.graph;
	JointSim* joints = graph->colors[OverflowIndex].jointSims.Data();
	int jointCount = (I32)graph->colors[OverflowIndex].jointSims.Size();

	for (int i = 0; i < jointCount; ++i)
	{
		JointSim& joint = joints[i];
		WarmStartJoint(joint, context);
	}
}

void Physics::WarmStartJointsTask(int startIndex, int endIndex, StepContext& context, int colorIndex)
{
	GraphColor* color = context.graph->colors + colorIndex;
	JointSim* joints = color->jointSims.Data();

	for (int i = startIndex; i < endIndex; ++i)
	{
		JointSim* joint = joints + i;
		WarmStartJoint(*joint, context);
	}
}

void Physics::WarmStartJoint(JointSim& joint, StepContext& context)
{
	switch (joint.type)
	{
	case JOINT_TYPE_DISTANCE: WarmStartDistanceJoint(joint, context); break;
	case JOINT_TYPE_MOTOR: WarmStartMotorJoint(joint, context); break;
	case JOINT_TYPE_MOUSE: WarmStartMouseJoint(joint, context); break;
	case JOINT_TYPE_PRISMATIC: WarmStartPrismaticJoint(joint, context); break;
	case JOINT_TYPE_REVOLUTE: WarmStartRevoluteJoint(joint, context); break;
	case JOINT_TYPE_WELD: WarmStartWeldJoint(joint, context); break;
	case JOINT_TYPE_WHEEL: WarmStartWheelJoint(joint, context); break;
	default: break;
	}
}

void Physics::WarmStartDistanceJoint(JointSim& base, StepContext& context)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	DistanceJoint& joint = base.distanceJoint;
	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	Vector2 ds = (stateB.deltaPosition - stateA.deltaPosition) + (rB - rA);
	Vector2 separation = joint.deltaCenter + ds;
	Vector2 axis = separation.Normalized();

	float axialImpulse = joint.impulse + joint.lowerImpulse - joint.upperImpulse + joint.motorImpulse;
	Vector2 P = axis * axialImpulse;

	stateA.linearVelocity = stateA.linearVelocity - P * mA;
	stateA.angularVelocity -= iA * rA.Cross(P);
	stateB.linearVelocity = stateB.linearVelocity + P * mB;
	stateB.angularVelocity += iB * rB.Cross(P);
}

void Physics::WarmStartMotorJoint(JointSim& base, StepContext& context)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	MotorJoint* joint = &base.motorJoint;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	BodyState& bodyA = joint->indexA == NullIndex ? dummyState : context.states[joint->indexA];
	BodyState& bodyB = joint->indexB == NullIndex ? dummyState : context.states[joint->indexB];

	Vector2 rA = joint->anchorA * bodyA.deltaRotation;
	Vector2 rB = joint->anchorB * bodyB.deltaRotation;

	bodyA.linearVelocity = bodyA.linearVelocity - joint->linearImpulse * mA;
	bodyA.angularVelocity -= iA * (rA.Cross(joint->linearImpulse) + joint->angularImpulse);
	bodyB.linearVelocity = bodyB.linearVelocity + joint->linearImpulse * mB;
	bodyB.angularVelocity += iB * (rB.Cross(joint->linearImpulse) + joint->angularImpulse);
}

void Physics::WarmStartMouseJoint(JointSim& base, StepContext& context)
{
	float mB = base.invMassB;
	float iB = base.invIB;

	MouseJoint& joint = base.mouseJoint;

	BodyState& stateB = context.states[joint.indexB];
	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	Quaternion2 dqB = stateB.deltaRotation;
	Vector2 rB = joint.anchorB * dqB;

	vB = vB + joint.linearImpulse * mB;
	wB += iB * (rB.Cross(joint.linearImpulse) + joint.angularImpulse);

	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::WarmStartPrismaticJoint(JointSim& base, StepContext& context)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	PrismaticJoint& joint = base.prismaticJoint;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	Vector2 d = ((stateB.deltaPosition - stateA.deltaPosition) + joint.deltaCenter) + (rB - rA);
	Vector2 axisA = joint.axisA * stateA.deltaRotation;

	// impulse is applied at anchor point on body B
	float a1 = (d + rA).Cross(axisA);
	float a2 = rB.Cross(axisA);
	float axialImpulse = joint.springImpulse + joint.motorImpulse + joint.lowerImpulse - joint.upperImpulse;

	// perpendicular constraint
	Vector2 perpA = axisA.PerpendicularLeft();
	float s1 = (d + rA).Cross(perpA);
	float s2 = rB.Cross(perpA);
	float perpImpulse = joint.impulse.x;
	float angleImpulse = joint.impulse.y;

	Vector2 P = axisA * axialImpulse + perpA * perpImpulse;
	float LA = axialImpulse * a1 + perpImpulse * s1 + angleImpulse;
	float LB = axialImpulse * a2 + perpImpulse * s2 + angleImpulse;

	stateA.linearVelocity = stateA.linearVelocity - P * mA;
	stateA.angularVelocity -= iA * LA;
	stateB.linearVelocity = stateB.linearVelocity + P * mB;
	stateB.angularVelocity += iB * LB;
}

void Physics::WarmStartRevoluteJoint(JointSim& base, StepContext& context)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	RevoluteJoint& joint = base.revoluteJoint;
	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	float axialImpulse = joint.springImpulse + joint.motorImpulse + joint.lowerImpulse - joint.upperImpulse;

	stateA.linearVelocity = stateA.linearVelocity - joint.linearImpulse * mA;
	stateA.angularVelocity -= iA * (rA.Cross(joint.linearImpulse) + axialImpulse);

	stateB.linearVelocity = stateB.linearVelocity + joint.linearImpulse * mB;
	stateB.angularVelocity += iB * (rB.Cross(joint.linearImpulse) + axialImpulse);
}

void Physics::WarmStartWeldJoint(JointSim& base, StepContext& context)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	WeldJoint& joint = base.weldJoint;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	stateA.linearVelocity = stateA.linearVelocity - joint.linearImpulse * mA;
	stateA.angularVelocity -= iA * (rA.Cross(joint.linearImpulse) + joint.angularImpulse);

	stateB.linearVelocity = stateB.linearVelocity + joint.linearImpulse * mB;
	stateB.angularVelocity += iB * (rB.Cross(joint.linearImpulse) + joint.angularImpulse);
}

void Physics::WarmStartWheelJoint(JointSim& base, StepContext& context)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	WheelJoint& joint = base.wheelJoint;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	Vector2 d = ((stateB.deltaPosition - stateA.deltaPosition) + joint.deltaCenter) + (rB - rA);
	Vector2 axisA = joint.axisA * stateA.deltaRotation;
	Vector2 perpA = axisA.PerpendicularLeft();

	float a1 = (d + rA).Cross(axisA);
	float a2 = rB.Cross(axisA);
	float s1 = (d + rA).Cross(perpA);
	float s2 = rB.Cross(perpA);

	float axialImpulse = joint.springImpulse + joint.lowerImpulse - joint.upperImpulse;

	Vector2 P = axisA * axialImpulse + perpA * joint.perpImpulse;
	float LA = axialImpulse * a1 + joint.perpImpulse * s1 + joint.motorImpulse;
	float LB = axialImpulse * a2 + joint.perpImpulse * s2 + joint.motorImpulse;

	stateA.linearVelocity = stateA.linearVelocity - P * mA;
	stateA.angularVelocity -= iA * LA;
	stateB.linearVelocity = stateB.linearVelocity + P * mB;
	stateB.angularVelocity += iB * LB;
}

void Physics::SolveOverflowJoints(StepContext& context, bool useBias)
{
	ConstraintGraph* graph = context.graph;
	JointSim* joints = graph->colors[OverflowIndex].jointSims.Data();
	int jointCount = (I32)graph->colors[OverflowIndex].jointSims.Size();

	for (int i = 0; i < jointCount; ++i)
	{
		JointSim& joint = joints[i];
		SolveJoint(joint, context, useBias);
	}
}

void Physics::SolveJointsTask(int startIndex, int endIndex, StepContext& context, int colorIndex, bool useBias)
{
	GraphColor& color = context.graph->colors[colorIndex];
	JointSim* joints = color.jointSims.Data();

	for (int i = startIndex; i < endIndex; ++i)
	{
		JointSim& joint = joints[i];
		SolveJoint(joint, context, useBias);
	}
}

void Physics::SolveJoint(JointSim& joint, StepContext& context, bool useBias)
{
	switch (joint.type)
	{
	case JOINT_TYPE_DISTANCE: SolveDistanceJoint(joint, context, useBias); break;
	case JOINT_TYPE_MOTOR: SolveMotorJoint(joint, context, useBias); break;
	case JOINT_TYPE_MOUSE: SolveMouseJoint(joint, context); break;
	case JOINT_TYPE_PRISMATIC: SolvePrismaticJoint(joint, context, useBias); break;
	case JOINT_TYPE_REVOLUTE: SolveRevoluteJoint(joint, context, useBias); break;
	case JOINT_TYPE_WELD: SolveWeldJoint(joint, context, useBias); break;
	case JOINT_TYPE_WHEEL: SolveWheelJoint(joint, context, useBias); break;
	default: break;
	}
}

void Physics::SolveDistanceJoint(JointSim& base, StepContext& context, bool useBias)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	DistanceJoint& joint = base.distanceJoint;
	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 vA = stateA.linearVelocity;
	float wA = stateA.angularVelocity;
	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	// current anchors
	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	// current separation
	Vector2 ds = (stateB.deltaPosition - stateA.deltaPosition) + (rB - rA);
	Vector2 separation = joint.deltaCenter + ds;

	float length = separation.Magnitude();
	Vector2 axis = separation.Normalized();

	// joint is soft if
	// - spring is enabled
	// - and (joint limit is disabled or limits are not equal)
	if (joint.enableSpring && (joint.minLength < joint.maxLength || joint.enableLimit == false))
	{
		// spring
		if (joint.hertz > 0.0f)
		{
			// Cdot = dot(u, v + cross(w, r))
			Vector2 vr = (vB - vA) + (rB.CrossInv(wB) - rA.CrossInv(wA));
			float Cdot = axis.Dot(vr);
			float C = length - joint.length;
			float bias = joint.distanceSoftness.biasRate * C;

			float m = joint.distanceSoftness.massScale * joint.axialMass;
			float impulse = -m * (Cdot + bias) - joint.distanceSoftness.impulseScale * joint.impulse;
			joint.impulse += impulse;

			Vector2 P = axis * impulse;
			vA = vA - P * mA;
			wA -= iA * rA.Cross(P);
			vB = vB + P * mB;
			wB += iB * rB.Cross(P);
		}

		if (joint.enableLimit)
		{
			// lower limit
			{
				Vector2 vr = (vB - vA) + (rB.CrossInv(wB) - rA.CrossInv(wA));
				float Cdot = axis.Dot(vr);

				float C = length - joint.minLength;

				float bias = 0.0f;
				float massCoeff = 1.0f;
				float impulseCoeff = 0.0f;
				if (C > 0.0f)
				{
					// speculative
					bias = C * context.inv_h;
				}
				else if (useBias)
				{
					bias = context.jointSoftness.biasRate * C;
					massCoeff = context.jointSoftness.massScale;
					impulseCoeff = context.jointSoftness.impulseScale;
				}

				float impulse = -massCoeff * joint.axialMass * (Cdot + bias) - impulseCoeff * joint.lowerImpulse;
				float newImpulse = Math::Max(0.0f, joint.lowerImpulse + impulse);
				impulse = newImpulse - joint.lowerImpulse;
				joint.lowerImpulse = newImpulse;

				Vector2 P = axis * impulse;
				vA = vA - P * mA;
				wA -= iA * rA.Cross(P);
				vB = vB + P * mB;
				wB += iB * rB.Cross(P);
			}

			// upper
			{
				Vector2 vr = (vA - vB) + (rA.CrossInv(wA) - rB.CrossInv(wB));
				float Cdot = axis.Dot(vr);

				float C = joint.maxLength - length;

				float bias = 0.0f;
				float massScale = 1.0f;
				float impulseScale = 0.0f;
				if (C > 0.0f)
				{
					// speculative
					bias = C * context.inv_h;
				}
				else if (useBias)
				{
					bias = context.jointSoftness.biasRate * C;
					massScale = context.jointSoftness.massScale;
					impulseScale = context.jointSoftness.impulseScale;
				}

				float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.upperImpulse;
				float newImpulse = Math::Max(0.0f, joint.upperImpulse + impulse);
				impulse = newImpulse - joint.upperImpulse;
				joint.upperImpulse = newImpulse;

				Vector2 P = axis * -impulse;
				vA = vA - P * mA;
				wA -= iA * rA.Cross(P);
				vB = vB + P * mB;
				wB += iB * rB.Cross(P);
			}
		}

		if (joint.enableMotor)
		{
			Vector2 vr = (vB - vA) + (rB.CrossInv(wB) - rA.CrossInv(wA));
			float Cdot = axis.Dot(vr);
			float impulse = joint.axialMass * (joint.motorSpeed - Cdot);
			float oldImpulse = joint.motorImpulse;
			float maxImpulse = context.h * joint.maxMotorForce;
			joint.motorImpulse = Math::Clamp(joint.motorImpulse + impulse, -maxImpulse, maxImpulse);
			impulse = joint.motorImpulse - oldImpulse;

			Vector2 P = axis * impulse;
			vA = vA - P * mA;
			wA -= iA * rA.Cross(P);
			vB = vB + P * mB;
			wB += iB * rB.Cross(P);
		}
	}
	else
	{
		// rigid constraint
		Vector2 vr = (vB - vA) + (rB.CrossInv(wB) - rA.CrossInv(wA));
		float Cdot = axis.Dot(vr);

		float C = length - joint.length;

		float bias = 0.0f;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias)
		{
			bias = context.jointSoftness.biasRate * C;
			massScale = context.jointSoftness.massScale;
			impulseScale = context.jointSoftness.impulseScale;
		}

		float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.impulse;
		joint.impulse += impulse;

		Vector2 P = axis * impulse;
		vA = vA - P * mA;
		wA -= iA * rA.Cross(P);
		vB = vB + P * mB;
		wB += iB * rB.Cross(P);
	}

	stateA.linearVelocity = vA;
	stateA.angularVelocity = wA;
	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::SolveMotorJoint(JointSim& base, StepContext& context, bool useBias)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	MotorJoint& joint = base.motorJoint;
	BodyState& bodyA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& bodyB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 vA = bodyA.linearVelocity;
	float wA = bodyA.angularVelocity;
	Vector2 vB = bodyB.linearVelocity;
	float wB = bodyB.angularVelocity;

	// angular constraint
	{
		float angularSeperation = bodyB.deltaRotation.RelativeAngle(bodyA.deltaRotation) + joint.deltaAngle;
		angularSeperation = Math::UnwindAngle(angularSeperation);

		float angularBias = context.inv_h * joint.correctionFactor * angularSeperation;

		float Cdot = wB - wA;
		float impulse = -joint.angularMass * (Cdot + angularBias);

		float oldImpulse = joint.angularImpulse;
		float maxImpulse = context.h * joint.maxTorque;
		joint.angularImpulse = Math::Clamp(joint.angularImpulse + impulse, -maxImpulse, maxImpulse);
		impulse = joint.angularImpulse - oldImpulse;

		wA -= iA * impulse;
		wB += iB * impulse;
	}

	// linear constraint
	{
		Vector2 rA = joint.anchorA * bodyA.deltaRotation;
		Vector2 rB = joint.anchorB * bodyB.deltaRotation;

		Vector2 ds = (bodyB.deltaPosition - bodyA.deltaPosition) + (rB - rA);
		Vector2 linearSeparation = joint.deltaCenter + ds;
		Vector2 linearBias = context.inv_h * joint.correctionFactor * linearSeparation;

		Vector2 Cdot = (vB + rB.CrossInv(wB)) - (vA + rA.CrossInv(wA));
		Vector2 b = joint.linearMass * (Cdot + linearBias);
		Vector2 impulse = -b;

		Vector2 oldImpulse = joint.linearImpulse;
		float maxImpulse = context.h * joint.maxForce;
		joint.linearImpulse = joint.linearImpulse + impulse;

		if (joint.linearImpulse.SqrMagnitude() > maxImpulse * maxImpulse)
		{
			joint.linearImpulse = joint.linearImpulse.Normalized();
			joint.linearImpulse.x *= maxImpulse;
			joint.linearImpulse.y *= maxImpulse;
		}

		impulse = joint.linearImpulse - oldImpulse;

		vA = vA - impulse * mA;
		wA -= iA * rA.Cross(impulse);
		vB = vB + impulse * mB;
		wB += iB * rB.Cross(impulse);
	}

	bodyA.linearVelocity = vA;
	bodyA.angularVelocity = wA;
	bodyB.linearVelocity = vB;
	bodyB.angularVelocity = wB;
}

void Physics::SolveMouseJoint(JointSim& base, StepContext& context)
{
	float mB = base.invMassB;
	float iB = base.invIB;

	MouseJoint& joint = base.mouseJoint;
	BodyState& stateB = context.states[joint.indexB];

	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	// Softness with no bias to reduce rotation speed
	{
		float massScale = joint.angularSoftness.massScale;
		float impulseScale = joint.angularSoftness.impulseScale;

		float impulse = iB > 0.0f ? -wB / iB : 0.0f;
		impulse = massScale * impulse - impulseScale * joint.angularImpulse;
		joint.angularImpulse += impulse;

		wB += iB * impulse;
	}

	float maxImpulse = joint.maxForce * context.h;

	{
		Quaternion2 dqB = stateB.deltaRotation;
		Vector2 rB = joint.anchorB * dqB;
		Vector2 Cdot = vB + rB.CrossInv(wB);

		Vector2 separation = stateB.deltaPosition + rB + joint.deltaCenter;
		Vector2 bias = separation * joint.linearSoftness.biasRate;

		float massScale = joint.linearSoftness.massScale;
		float impulseScale = joint.linearSoftness.impulseScale;

		Vector2 b = (Cdot + bias) * joint.linearMass;

		Vector2 impulse;
		impulse.x = -massScale * b.x - impulseScale * joint.linearImpulse.x;
		impulse.y = -massScale * b.y - impulseScale * joint.linearImpulse.y;

		Vector2 oldImpulse = joint.linearImpulse;
		joint.linearImpulse.x += impulse.x;
		joint.linearImpulse.y += impulse.y;

		float mag = joint.linearImpulse.Magnitude();
		if (mag > maxImpulse)
		{
			joint.linearImpulse = joint.linearImpulse.Normalized() * maxImpulse;
		}

		impulse.x = joint.linearImpulse.x - oldImpulse.x;
		impulse.y = joint.linearImpulse.y - oldImpulse.y;

		vB = vB + impulse * mB;
		wB += iB * rB.Cross(impulse);
	}

	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::SolvePrismaticJoint(JointSim& base, StepContext& context, bool useBias)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	PrismaticJoint& joint = base.prismaticJoint;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 vA = stateA.linearVelocity;
	float wA = stateA.angularVelocity;
	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	// current anchors
	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	Vector2 d = ((stateB.deltaPosition - stateA.deltaPosition) + joint.deltaCenter) + (rB - rA);
	Vector2 axisA = joint.axisA * stateA.deltaRotation;
	float translation = axisA.Dot(d);

	// These scalars are for torques generated by axial forces
	float a1 = (d + rA).Cross(axisA);
	float a2 = rB.Cross(axisA);

	// spring constraint
	if (joint.enableSpring)
	{
		// This is a real spring and should be applied even during relax
		float C = translation;
		float bias = joint.springSoftness.biasRate * C;
		float massScale = joint.springSoftness.massScale;
		float impulseScale = joint.springSoftness.impulseScale;

		float Cdot = axisA.Dot(vB - vA) + a2 * wB - a1 * wA;
		float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.springImpulse;
		joint.springImpulse += impulse;

		Vector2 P = axisA * impulse;
		float LA = impulse * a1;
		float LB = impulse * a2;

		vA = vA - P * mA;
		wA -= iA * LA;
		vB = vB + P * mB;
		wB += iB * LB;
	}

	// Solve motor constraint
	if (joint.enableMotor)
	{
		float Cdot = axisA.Dot(vB - vA) + a2 * wB - a1 * wA;
		float impulse = joint.axialMass * (joint.motorSpeed - Cdot);
		float oldImpulse = joint.motorImpulse;
		float maxImpulse = context.h * joint.maxMotorForce;
		joint.motorImpulse = Math::Clamp(joint.motorImpulse + impulse, -maxImpulse, maxImpulse);
		impulse = joint.motorImpulse - oldImpulse;

		Vector2 P = axisA * impulse;
		float LA = impulse * a1;
		float LB = impulse * a2;

		vA = vA - P * mA;
		wA -= iA * LA;
		vB = vB + P * mB;
		wB += iB * LB;
	}

	if (joint.enableLimit)
	{
		// Lower limit
		{
			float C = translation - joint.lowerTranslation;
			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;

			if (C > 0.0f)
			{
				// speculation
				bias = C * context.inv_h;
			}
			else if (useBias)
			{
				bias = context.jointSoftness.biasRate * C;
				massScale = context.jointSoftness.massScale;
				impulseScale = context.jointSoftness.impulseScale;
			}

			float oldImpulse = joint.lowerImpulse;
			float Cdot = axisA.Dot(vB - vA) + a2 * wB - a1 * wA;
			float impulse = -joint.axialMass * massScale * (Cdot + bias) - impulseScale * oldImpulse;
			joint.lowerImpulse = Math::Max(oldImpulse + impulse, 0.0f);
			impulse = joint.lowerImpulse - oldImpulse;

			Vector2 P = axisA * impulse;
			float LA = impulse * a1;
			float LB = impulse * a2;

			vA = vA - P * mA;
			wA -= iA * LA;
			vB = vB + P * mB;
			wB += iB * LB;
		}

		// Upper limit
		// Note: signs are flipped to keep C positive when the constraint is satisfied.
		// This also keeps the impulse positive when the limit is active.
		{
			// sign flipped
			float C = joint.upperTranslation - translation;
			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;

			if (C > 0.0f)
			{
				// speculation
				bias = C * context.inv_h;
			}
			else if (useBias)
			{
				bias = context.jointSoftness.biasRate * C;
				massScale = context.jointSoftness.massScale;
				impulseScale = context.jointSoftness.impulseScale;
			}

			float oldImpulse = joint.upperImpulse;
			// sign flipped
			float Cdot = axisA.Dot(vA - vB) + a1 * wA - a2 * wB;
			float impulse = -joint.axialMass * massScale * (Cdot + bias) - impulseScale * oldImpulse;
			joint.upperImpulse = Math::Max(oldImpulse + impulse, 0.0f);
			impulse = joint.upperImpulse - oldImpulse;

			Vector2 P = axisA * impulse;
			float LA = impulse * a1;
			float LB = impulse * a2;

			// sign flipped
			vA = vA + P * mA;
			wA += iA * LA;
			vB = vB - P * mB;
			wB -= iB * LB;
		}
	}

	// Solve the prismatic constraint in block form
	{
		Vector2 perpA = axisA.PerpendicularLeft();

		// These scalars are for torques generated by the perpendicular constraint force
		float s1 = (d + rA).Cross(perpA);
		float s2 = rB.Cross(perpA);

		Vector2 Cdot;
		Cdot.x = perpA.Dot(vB - vA) + s2 * wB - s1 * wA;
		Cdot.y = wB - wA;

		Vector2 bias = Vector2Zero;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias)
		{
			Vector2 C;
			C.x = perpA.Dot(d);
			C.y = stateB.deltaRotation.RelativeAngle(stateA.deltaRotation) + joint.deltaAngle;

			bias = C * context.jointSoftness.biasRate;
			massScale = context.jointSoftness.massScale;
			impulseScale = context.jointSoftness.impulseScale;
		}

		float k11 = mA + mB + iA * s1 * s1 + iB * s2 * s2;
		float k12 = iA * s1 + iB * s2;
		float k22 = iA + iB;
		if (k22 == 0.0f)
		{
			// For bodies with fixed rotation.
			k22 = 1.0f;
		}

		Matrix2 K = { { k11, k12 }, { k12, k22 } };

		Vector2 b = K.Solve(Cdot + bias);
		Vector2 impulse;
		impulse.x = -massScale * b.x - impulseScale * joint.impulse.x;
		impulse.y = -massScale * b.y - impulseScale * joint.impulse.y;

		joint.impulse.x += impulse.x;
		joint.impulse.y += impulse.y;

		Vector2 P = perpA * impulse.x;
		float LA = impulse.x * s1 + impulse.y;
		float LB = impulse.x * s2 + impulse.y;

		vA = vA - P * mA;
		wA -= iA * LA;
		vB = vB + P * mB;
		wB += iB * LB;
	}

	stateA.linearVelocity = vA;
	stateA.angularVelocity = wA;
	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::SolveRevoluteJoint(JointSim& base, StepContext& context, bool useBias)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	RevoluteJoint& joint = base.revoluteJoint;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 vA = stateA.linearVelocity;
	float wA = stateA.angularVelocity;
	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	bool fixedRotation = (iA + iB == 0.0f);
	// const float maxBias = context->maxBiasVelocity;

	// Solve spring.
	if (joint.enableSpring && fixedRotation == false)
	{
		float C = stateB.deltaRotation.RelativeAngle(stateA.deltaRotation) + joint.deltaAngle;
		float bias = joint.springSoftness.biasRate * C;
		float massScale = joint.springSoftness.massScale;
		float impulseScale = joint.springSoftness.impulseScale;

		float Cdot = wB - wA;
		float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.springImpulse;
		joint.springImpulse += impulse;

		wA -= iA * impulse;
		wB += iB * impulse;
	}

	// Solve motor constraint.
	if (joint.enableMotor && fixedRotation == false)
	{
		float Cdot = wB - wA - joint.motorSpeed;
		float impulse = -joint.axialMass * Cdot;
		float oldImpulse = joint.motorImpulse;
		float maxImpulse = context.h * joint.maxMotorTorque;
		joint.motorImpulse = Math::Clamp(joint.motorImpulse + impulse, -maxImpulse, maxImpulse);
		impulse = joint.motorImpulse - oldImpulse;

		wA -= iA * impulse;
		wB += iB * impulse;
	}

	if (joint.enableLimit && fixedRotation == false)
	{
		float jointAngle = stateB.deltaRotation.RelativeAngle(stateA.deltaRotation) + joint.deltaAngle;
		jointAngle = Math::UnwindAngle(jointAngle);

		// Lower limit
		{
			float C = jointAngle - joint.lowerAngle;
			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;
			if (C > 0.0f)
			{
				// speculation
				bias = C * context.inv_h;
			}
			else if (useBias)
			{
				bias = context.jointSoftness.biasRate * C;
				massScale = context.jointSoftness.massScale;
				impulseScale = context.jointSoftness.impulseScale;
			}

			float Cdot = wB - wA;
			float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.lowerImpulse;
			float oldImpulse = joint.lowerImpulse;
			joint.lowerImpulse = Math::Max(joint.lowerImpulse + impulse, 0.0f);
			impulse = joint.lowerImpulse - oldImpulse;

			wA -= iA * impulse;
			wB += iB * impulse;
		}

		// Upper limit
		// Note: signs are flipped to keep C positive when the constraint is satisfied.
		// This also keeps the impulse positive when the limit is active.
		{
			float C = joint.upperAngle - jointAngle;
			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;
			if (C > 0.0f)
			{
				// speculation
				bias = C * context.inv_h;
			}
			else if (useBias)
			{
				bias = context.jointSoftness.biasRate * C;
				massScale = context.jointSoftness.massScale;
				impulseScale = context.jointSoftness.impulseScale;
			}

			// sign flipped on Cdot
			float Cdot = wA - wB;
			float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.lowerImpulse;
			float oldImpulse = joint.upperImpulse;
			joint.upperImpulse = Math::Max(joint.upperImpulse + impulse, 0.0f);
			impulse = joint.upperImpulse - oldImpulse;

			// sign flipped on applied impulse
			wA += iA * impulse;
			wB -= iB * impulse;
		}
	}

	// Solve point-to-point constraint
	{
		// J = [-I -r1_skew I r2_skew]
		// r_skew = [-ry; rx]
		// K = [ mA+r1y^2*iA+mB+r2y^2*iB,  -r1y*iA*r1x-r2y*iB*r2x]
		//     [  -r1y*iA*r1x-r2y*iB*r2x, mA+r1x^2*iA+mB+r2x^2*iB]

		// current anchors
		Vector2 rA = joint.anchorA * stateA.deltaRotation;
		Vector2 rB = joint.anchorB * stateB.deltaRotation;

		Vector2 Cdot = (vB + rB.CrossInv(wB)) - (vA + rA.CrossInv(wA));

		Vector2 bias = Vector2Zero;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias)
		{
			Vector2 dcA = stateA.deltaPosition;
			Vector2 dcB = stateB.deltaPosition;

			Vector2 separation = ((dcB - dcA) + (rB - rA)) + joint.deltaCenter;
			bias = separation * context.jointSoftness.biasRate;
			massScale = context.jointSoftness.massScale;
			impulseScale = context.jointSoftness.impulseScale;
		}

		Matrix2 K;
		K.a.x = mA + mB + rA.y * rA.y * iA + rB.y * rB.y * iB;
		K.b.x = -rA.y * rA.x * iA - rB.y * rB.x * iB;
		K.a.y = K.b.x;
		K.b.y = mA + mB + rA.x * rA.x * iA + rB.x * rB.x * iB;
		Vector2 b = K.Solve(Cdot + bias);

		Vector2 impulse;
		impulse.x = -massScale * b.x - impulseScale * joint.linearImpulse.x;
		impulse.y = -massScale * b.y - impulseScale * joint.linearImpulse.y;
		joint.linearImpulse.x += impulse.x;
		joint.linearImpulse.y += impulse.y;

		vA = vA - impulse * mA;
		wA -= iA * rA.Cross(impulse);
		vB = vB + impulse * mB;
		wB += iB * rB.Cross(impulse);
	}

	stateA.linearVelocity = vA;
	stateA.angularVelocity = wA;
	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::SolveWeldJoint(JointSim& base, StepContext& context, bool useBias)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	WeldJoint& joint = base.weldJoint;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 vA = stateA.linearVelocity;
	float wA = stateA.angularVelocity;
	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	// angular constraint
	{
		float bias = 0.0f;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias || joint.angularHertz > 0.0f)
		{
			float C = stateB.deltaRotation.RelativeAngle(stateA.deltaRotation) + joint.deltaAngle;
			bias = joint.angularSoftness.biasRate * C;
			massScale = joint.angularSoftness.massScale;
			impulseScale = joint.angularSoftness.impulseScale;
		}

		float Cdot = wB - wA;
		float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.angularImpulse;
		joint.angularImpulse += impulse;

		wA -= iA * impulse;
		wB += iB * impulse;
	}

	// linear constraint
	{
		Vector2 rA = joint.anchorA * stateA.deltaRotation;
		Vector2 rB = joint.anchorB * stateB.deltaRotation;

		Vector2 bias = Vector2Zero;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias || joint.linearHertz > 0.0f)
		{
			Vector2 dcA = stateA.deltaPosition;
			Vector2 dcB = stateB.deltaPosition;
			Vector2 C = (((dcB - dcA) + (rB - rA)) + joint.deltaCenter);

			bias = C * joint.linearSoftness.biasRate;
			massScale = joint.linearSoftness.massScale;
			impulseScale = joint.linearSoftness.impulseScale;
		}

		Vector2 Cdot = (vB + rB.CrossInv(wB)) - (vA + rA.CrossInv(wA));

		Matrix2 K;
		K.a.x = mA + mB + rA.y * rA.y * iA + rB.y * rB.y * iB;
		K.b.x = -rA.y * rA.x * iA - rB.y * rB.x * iB;
		K.a.y = K.b.x;
		K.b.y = mA + mB + rA.x * rA.x * iA + rB.x * rB.x * iB;
		Vector2 b = K.Solve(Cdot + bias);

		Vector2 impulse = {
			-massScale * b.x - impulseScale * joint.linearImpulse.x,
			-massScale * b.y - impulseScale * joint.linearImpulse.y,
		};

		joint.linearImpulse = joint.linearImpulse + impulse;

		vA = vA - impulse * mA;
		wA -= iA * rA.Cross(impulse);
		vB = vB + impulse * mB;
		wB += iB * rB.Cross(impulse);
	}

	stateA.linearVelocity = vA;
	stateA.angularVelocity = wA;
	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::SolveWheelJoint(JointSim& base, StepContext& context, bool useBias)
{
	float mA = base.invMassA;
	float mB = base.invMassB;
	float iA = base.invIA;
	float iB = base.invIB;

	// dummy state for static bodies
	BodyState dummyState = BodyStateIdentity;

	WheelJoint& joint = base.wheelJoint;

	// This is a dummy body to represent a static body since static bodies don't have a solver body.
	BodyState dummyBody;

	BodyState& stateA = joint.indexA == NullIndex ? dummyState : context.states[joint.indexA];
	BodyState& stateB = joint.indexB == NullIndex ? dummyState : context.states[joint.indexB];

	Vector2 vA = stateA.linearVelocity;
	float wA = stateA.angularVelocity;
	Vector2 vB = stateB.linearVelocity;
	float wB = stateB.angularVelocity;

	bool fixedRotation = (iA + iB == 0.0f);

	// current anchors
	Vector2 rA = joint.anchorA * stateA.deltaRotation;
	Vector2 rB = joint.anchorB * stateB.deltaRotation;

	Vector2 d = ((stateB.deltaPosition - stateA.deltaPosition) + joint.deltaCenter) + (rB - rA);
	Vector2 axisA = joint.axisA * stateA.deltaRotation;
	float translation = axisA.Dot(d);

	float a1 = (d + rA).Cross(axisA);
	float a2 = rB.Cross(axisA);

	// motor constraint
	if (joint.enableMotor && fixedRotation == false)
	{
		float Cdot = wB - wA - joint.motorSpeed;
		float impulse = -joint.motorMass * Cdot;
		float oldImpulse = joint.motorImpulse;
		float maxImpulse = context.h * joint.maxMotorTorque;
		joint.motorImpulse = Math::Clamp(joint.motorImpulse + impulse, -maxImpulse, maxImpulse);
		impulse = joint.motorImpulse - oldImpulse;

		wA -= iA * impulse;
		wB += iB * impulse;
	}

	// spring constraint
	if (joint.enableSpring)
	{
		// This is a real spring and should be applied even during relax
		float C = translation;
		float bias = joint.springSoftness.biasRate * C;
		float massScale = joint.springSoftness.massScale;
		float impulseScale = joint.springSoftness.impulseScale;

		float Cdot = axisA.Dot(vB - vA) + a2 * wB - a1 * wA;
		float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.springImpulse;
		joint.springImpulse += impulse;

		Vector2 P = axisA * impulse;
		float LA = impulse * a1;
		float LB = impulse * a2;

		vA = vA - P * mA;
		wA -= iA * LA;
		vB = vB + P * mB;
		wB += iB * LB;
	}

	if (joint.enableLimit)
	{
		float translation = axisA.Dot(d);

		// Lower limit
		{
			float C = translation - joint.lowerTranslation;
			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;

			if (C > 0.0f)
			{
				// speculation
				bias = C * context.inv_h;
			}
			else if (useBias)
			{
				bias = context.jointSoftness.biasRate * C;
				massScale = context.jointSoftness.massScale;
				impulseScale = context.jointSoftness.impulseScale;
			}

			float Cdot = axisA.Dot(vB - vA) + a2 * wB - a1 * wA;
			float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.lowerImpulse;
			float oldImpulse = joint.lowerImpulse;
			joint.lowerImpulse = Math::Max(oldImpulse + impulse, 0.0f);
			impulse = joint.lowerImpulse - oldImpulse;

			Vector2 P = axisA * impulse;
			float LA = impulse * a1;
			float LB = impulse * a2;

			vA = vA - P * mA;
			wA -= iA * LA;
			vB = vB + P * mB;
			wB += iB * LB;
		}

		// Upper limit
		// Note: signs are flipped to keep C positive when the constraint is satisfied.
		// This also keeps the impulse positive when the limit is active.
		{
			// sign flipped
			float C = joint.upperTranslation - translation;
			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;

			if (C > 0.0f)
			{
				// speculation
				bias = C * context.inv_h;
			}
			else if (useBias)
			{
				bias = context.jointSoftness.biasRate * C;
				massScale = context.jointSoftness.massScale;
				impulseScale = context.jointSoftness.impulseScale;
			}

			// sign flipped on Cdot
			float Cdot = axisA.Dot(vA - vB) + a1 * wA - a2 * wB;
			float impulse = -massScale * joint.axialMass * (Cdot + bias) - impulseScale * joint.upperImpulse;
			float oldImpulse = joint.upperImpulse;
			joint.upperImpulse = Math::Max(oldImpulse + impulse, 0.0f);
			impulse = joint.upperImpulse - oldImpulse;

			Vector2 P = axisA * impulse;
			float LA = impulse * a1;
			float LB = impulse * a2;

			// sign flipped on applied impulse
			vA = vA + P * mA;
			wA += iA * LA;
			vB = vB - P * mB;
			wB -= iB * LB;
		}
	}

	// point to line constraint
	{
		Vector2 perpA = axisA.PerpendicularLeft();

		float bias = 0.0f;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias)
		{
			float C = perpA.Dot(d);
			bias = context.jointSoftness.biasRate * C;
			massScale = context.jointSoftness.massScale;
			impulseScale = context.jointSoftness.impulseScale;
		}

		float s1 = (d + rA).Cross(perpA);
		float s2 = rB.Cross(perpA);
		float Cdot = perpA.Dot(vB - vA) + s2 * wB - s1 * wA;

		float impulse = -massScale * joint.perpMass * (Cdot + bias) - impulseScale * joint.perpImpulse;
		joint.perpImpulse += impulse;

		Vector2 P = perpA * impulse;
		float LA = impulse * s1;
		float LB = impulse * s2;

		vA = vA - P * mA;
		wA -= iA * LA;
		vB = vB + P * mB;
		wB += iB * LB;
	}

	stateA.linearVelocity = vA;
	stateA.angularVelocity = wA;
	stateB.linearVelocity = vB;
	stateB.angularVelocity = wB;
}

void Physics::IntegrateVelocitiesTask(int startIndex, int endIndex, StepContext& context)
{
	BodyState* states = context.states;
	BodySim* sims = context.sims;

	float h = context.h;
	float maxLinearSpeed = context.maxLinearVelocity;
	float maxAngularSpeed = MaxRotation * context.inv_dt;
	float maxLinearSpeedSquared = maxLinearSpeed * maxLinearSpeed;
	float maxAngularSpeedSquared = maxAngularSpeed * maxAngularSpeed;

	for (int i = startIndex; i < endIndex; ++i)
	{
		BodySim& sim = sims[i];
		BodyState& state = states[i];

		Vector2 v = state.linearVelocity;
		float w = state.angularVelocity;

		// Apply forces, torque, gravity, and damping
		// Apply damping.
		// Differential equation: dv/dt + c * v = 0
		// Solution: v(t) = v0 * exp(-c * t)
		// Time step: v(t + dt) = v0 * exp(-c * (t + dt)) = v0 * exp(-c * t) * exp(-c * dt) = v(t) * exp(-c * dt)
		// v2 = exp(-c * dt) * v1
		// Pade approximation:
		// v2 = v1 * 1 / (1 + c * dt)
		float linearDamping = 1.0f / (1.0f + h * sim.linearDamping);
		float angularDamping = 1.0f / (1.0f + h * sim.angularDamping);
		Vector2 linearVelocityDelta = (sim.force + gravity * sim.mass * sim.gravityScale) * h * sim.invMass;
		float angularVelocityDelta = h * sim.invInertia * sim.torque;

		v = linearVelocityDelta + v * linearDamping;
		w = angularVelocityDelta + angularDamping * w;

		// Clamp to max linear speed
		if (v.Dot(v) > maxLinearSpeedSquared)
		{
			float ratio = maxLinearSpeed / v.Magnitude();
			v *= ratio;
			sim.isSpeedCapped = true;
		}

		// Clamp to max angular speed
		if (w * w > maxAngularSpeedSquared && sim.allowFastRotation == false)
		{
			float ratio = maxAngularSpeed / Math::Abs(w);
			w *= ratio;
			sim.isSpeedCapped = true;
		}

		state.linearVelocity = v;
		state.angularVelocity = w;
	}
}

void Physics::IntegratePositionsTask(int startIndex, int endIndex, StepContext& context)
{
	BodyState* states = context.states;
	F32 h = context.h;

	for (int i = startIndex; i < endIndex; ++i)
	{
		BodyState& state = states[i];
		state.deltaRotation = state.deltaRotation.Integrate(h * state.angularVelocity);
		state.deltaPosition = state.deltaPosition + state.linearVelocity * h;
	}
}

void Physics::ApplyRestitutionTask(int startIndex, int endIndex, StepContext& context, int colorIndex)
{
	BodyState* states = context.states;
	ContactConstraintSIMD* constraints = context.graph->colors[colorIndex].simdConstraints;
	FloatW threshold = SplatW(restitutionThreshold);
	FloatW zero = ZeroFloatW;

	for (int i = startIndex; i < endIndex; ++i)
	{
		ContactConstraintSIMD& c = constraints[i];

		SimdBody bA = GatherBodies(states, c.indexA);
		SimdBody bB = GatherBodies(states, c.indexB);

		// first point non-penetration constraint
		{
			// Set effective mass to zero if restitution should not be applied
			FloatW mask1 = (c.relativeVelocity1 + threshold) > zero;
			FloatW mask2 = c.maxNormalImpulse1 == zero;
			FloatW mask = mask1 | mask2;
			FloatW mass = BlendW(c.normalMass1, zero, mask);

			// fixed anchors for Jacobians
			Vector2W rA = c.anchorA1;
			Vector2W rB = c.anchorB1;

			// Relative velocity at contact
			FloatW dvx = (bB.v.x - bB.w * rB.y) - (bA.v.x - bA.w * rA.y);
			FloatW dvy = (bB.v.y + bB.w * rB.x) - (bA.v.y + bA.w * rA.x);
			FloatW vn = dvx * c.normal.x + dvy * c.normal.y;

			// Compute normal impulse
			FloatW negImpulse = mass * (vn + c.restitution * c.relativeVelocity1);

			// Clamp the accumulated impulse
			FloatW newImpulse = MaxW(c.normalImpulse1 - negImpulse, ZeroFloatW);
			FloatW impulse = newImpulse - c.normalImpulse1;
			c.normalImpulse1 = newImpulse;

			// Apply contact impulse
			FloatW Px = impulse * c.normal.x;
			FloatW Py = impulse * c.normal.y;

			bA.v.x = MulSubW(bA.v.x, c.invMassA, Px);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, Py);
			bA.w = MulSubW(bA.w, c.invIA, (rA.x * Py - rA.y * Px));

			bB.v.x = MulAddW(bB.v.x, c.invMassB, Px);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, Py);
			bB.w = MulAddW(bB.w, c.invIB, (rB.x * Py - rB.y * Px));
		}

		// second point non-penetration constraint
		{
			// Set effective mass to zero if restitution should not be applied
			FloatW mask1 = (c.relativeVelocity2 + threshold) > zero;
			FloatW mask2 = c.maxNormalImpulse2 == zero;
			FloatW mask = mask1 | mask2;
			FloatW mass = BlendW(c.normalMass2, zero, mask);

			// fixed anchors for Jacobians
			Vector2W rA = c.anchorA2;
			Vector2W rB = c.anchorB2;

			// Relative velocity at contact
			FloatW dvx = (bB.v.x - bB.w * rB.y) - (bA.v.x - bA.w * rA.y);
			FloatW dvy = (bB.v.y + bB.w * rB.x) - (bA.v.y + bA.w * rA.x);
			FloatW vn = dvx * c.normal.x + dvy * c.normal.y;

			// Compute normal impulse
			FloatW negImpulse = mass * (vn + c.restitution * c.relativeVelocity2);

			// Clamp the accumulated impulse
			FloatW newImpulse = MaxW(c.normalImpulse2 - negImpulse, ZeroFloatW);
			FloatW impulse = newImpulse - c.normalImpulse2;
			c.normalImpulse2 = newImpulse;

			// Apply contact impulse
			FloatW Px = impulse * c.normal.x;
			FloatW Py = impulse * c.normal.y;

			bA.v.x = MulSubW(bA.v.x, c.invMassA, Px);
			bA.v.y = MulSubW(bA.v.y, c.invMassA, Py);
			bA.w = MulSubW(bA.w, c.invIA, rA.x * Py - rA.y * Px);

			bB.v.x = MulAddW(bB.v.x, c.invMassB, Px);
			bB.v.y = MulAddW(bB.v.y, c.invMassB, Py);
			bB.w = MulAddW(bB.w, c.invIB, rB.x * Py - rB.y * Px);
		}

		ScatterBodies(states, c.indexA, bA);
		ScatterBodies(states, c.indexB, bB);
	}
}

#if NH_SIMD_WIDTH == 8

void Physics::StoreImpulsesTask(int startIndex, int endIndex, StepContext& context)
{
	Vector<ContactSim*> contacts = context.contacts;
	const ContactConstraintSIMD* constraints = context.simdContactConstraints;

	Manifold dummy{};

	for (int i = startIndex; i < endIndex; ++i)
	{
		const ContactConstraintSIMD& c = constraints[i];
		const float* normalImpulse1 = (float*)&c.normalImpulse1;
		const float* normalImpulse2 = (float*)&c.normalImpulse2;
		const float* tangentImpulse1 = (float*)&c.tangentImpulse1;
		const float* tangentImpulse2 = (float*)&c.tangentImpulse2;
		const float* maxNormalImpulse1 = (float*)&c.maxNormalImpulse1;
		const float* maxNormalImpulse2 = (float*)&c.maxNormalImpulse2;
		const float* normalVelocity1 = (float*)&c.relativeVelocity1;
		const float* normalVelocity2 = (float*)&c.relativeVelocity2;

		int base = 8 * i;
		Manifold* m0 = contacts[base + 0] == NULL ? &dummy : &contacts[base + 0]->manifold;
		Manifold* m1 = contacts[base + 1] == NULL ? &dummy : &contacts[base + 1]->manifold;
		Manifold* m2 = contacts[base + 2] == NULL ? &dummy : &contacts[base + 2]->manifold;
		Manifold* m3 = contacts[base + 3] == NULL ? &dummy : &contacts[base + 3]->manifold;
		Manifold* m4 = contacts[base + 4] == NULL ? &dummy : &contacts[base + 4]->manifold;
		Manifold* m5 = contacts[base + 5] == NULL ? &dummy : &contacts[base + 5]->manifold;
		Manifold* m6 = contacts[base + 6] == NULL ? &dummy : &contacts[base + 6]->manifold;
		Manifold* m7 = contacts[base + 7] == NULL ? &dummy : &contacts[base + 7]->manifold;

		m0->points[0].normalImpulse = normalImpulse1[0];
		m0->points[0].tangentImpulse = tangentImpulse1[0];
		m0->points[0].maxNormalImpulse = maxNormalImpulse1[0];
		m0->points[0].normalVelocity = normalVelocity1[0];

		m0->points[1].normalImpulse = normalImpulse2[0];
		m0->points[1].tangentImpulse = tangentImpulse2[0];
		m0->points[1].maxNormalImpulse = maxNormalImpulse2[0];
		m0->points[1].normalVelocity = normalVelocity2[0];

		m1->points[0].normalImpulse = normalImpulse1[1];
		m1->points[0].tangentImpulse = tangentImpulse1[1];
		m1->points[0].maxNormalImpulse = maxNormalImpulse1[1];
		m1->points[0].normalVelocity = normalVelocity1[1];

		m1->points[1].normalImpulse = normalImpulse2[1];
		m1->points[1].tangentImpulse = tangentImpulse2[1];
		m1->points[1].maxNormalImpulse = maxNormalImpulse2[1];
		m1->points[1].normalVelocity = normalVelocity2[1];

		m2->points[0].normalImpulse = normalImpulse1[2];
		m2->points[0].tangentImpulse = tangentImpulse1[2];
		m2->points[0].maxNormalImpulse = maxNormalImpulse1[2];
		m2->points[0].normalVelocity = normalVelocity1[2];

		m2->points[1].normalImpulse = normalImpulse2[2];
		m2->points[1].tangentImpulse = tangentImpulse2[2];
		m2->points[1].maxNormalImpulse = maxNormalImpulse2[2];
		m2->points[1].normalVelocity = normalVelocity2[2];

		m3->points[0].normalImpulse = normalImpulse1[3];
		m3->points[0].tangentImpulse = tangentImpulse1[3];
		m3->points[0].maxNormalImpulse = maxNormalImpulse1[3];
		m3->points[0].normalVelocity = normalVelocity1[3];

		m3->points[1].normalImpulse = normalImpulse2[3];
		m3->points[1].tangentImpulse = tangentImpulse2[3];
		m3->points[1].maxNormalImpulse = maxNormalImpulse2[3];
		m3->points[1].normalVelocity = normalVelocity2[3];

		m4->points[0].normalImpulse = normalImpulse1[4];
		m4->points[0].tangentImpulse = tangentImpulse1[4];
		m4->points[0].maxNormalImpulse = maxNormalImpulse1[4];
		m4->points[0].normalVelocity = normalVelocity1[4];

		m4->points[1].normalImpulse = normalImpulse2[4];
		m4->points[1].tangentImpulse = tangentImpulse2[4];
		m4->points[1].maxNormalImpulse = maxNormalImpulse2[4];
		m4->points[1].normalVelocity = normalVelocity2[4];

		m5->points[0].normalImpulse = normalImpulse1[5];
		m5->points[0].tangentImpulse = tangentImpulse1[5];
		m5->points[0].maxNormalImpulse = maxNormalImpulse1[5];
		m5->points[0].normalVelocity = normalVelocity1[5];

		m5->points[1].normalImpulse = normalImpulse2[5];
		m5->points[1].tangentImpulse = tangentImpulse2[5];
		m5->points[1].maxNormalImpulse = maxNormalImpulse2[5];
		m5->points[1].normalVelocity = normalVelocity2[5];

		m6->points[0].normalImpulse = normalImpulse1[6];
		m6->points[0].tangentImpulse = tangentImpulse1[6];
		m6->points[0].maxNormalImpulse = maxNormalImpulse1[6];
		m6->points[0].normalVelocity = normalVelocity1[6];

		m6->points[1].normalImpulse = normalImpulse2[6];
		m6->points[1].tangentImpulse = tangentImpulse2[6];
		m6->points[1].maxNormalImpulse = maxNormalImpulse2[6];
		m6->points[1].normalVelocity = normalVelocity2[6];

		m7->points[0].normalImpulse = normalImpulse1[7];
		m7->points[0].tangentImpulse = tangentImpulse1[7];
		m7->points[0].maxNormalImpulse = maxNormalImpulse1[7];
		m7->points[0].normalVelocity = normalVelocity1[7];

		m7->points[1].normalImpulse = normalImpulse2[7];
		m7->points[1].tangentImpulse = tangentImpulse2[7];
		m7->points[1].maxNormalImpulse = maxNormalImpulse2[7];
		m7->points[1].normalVelocity = normalVelocity2[7];
	}
}

#else

void Physics::StoreImpulsesTask(int startIndex, int endIndex, StepContext& context)
{
	Vector<ContactSim*> contacts = context.contacts;
	const ContactConstraintSIMD* constraints = context.simdContactConstraints;

	Manifold dummy{};

	for (int i = startIndex; i < endIndex; ++i)
	{
		const ContactConstraintSIMD& c = constraints[i];
		const float* normalImpulse1 = (float*)&c.normalImpulse1;
		const float* normalImpulse2 = (float*)&c.normalImpulse2;
		const float* tangentImpulse1 = (float*)&c.tangentImpulse1;
		const float* tangentImpulse2 = (float*)&c.tangentImpulse2;
		const float* maxNormalImpulse1 = (float*)&c.maxNormalImpulse1;
		const float* maxNormalImpulse2 = (float*)&c.maxNormalImpulse2;
		const float* normalVelocity1 = (float*)&c.relativeVelocity1;
		const float* normalVelocity2 = (float*)&c.relativeVelocity2;

		int base = 4 * i;
		Manifold* m0 = contacts[base + 0] == NULL ? &dummy : &contacts[base + 0]->manifold;
		Manifold* m1 = contacts[base + 1] == NULL ? &dummy : &contacts[base + 1]->manifold;
		Manifold* m2 = contacts[base + 2] == NULL ? &dummy : &contacts[base + 2]->manifold;
		Manifold* m3 = contacts[base + 3] == NULL ? &dummy : &contacts[base + 3]->manifold;

		m0->points[0].normalImpulse = normalImpulse1[0];
		m0->points[0].tangentImpulse = tangentImpulse1[0];
		m0->points[0].maxNormalImpulse = maxNormalImpulse1[0];
		m0->points[0].normalVelocity = normalVelocity1[0];

		m0->points[1].normalImpulse = normalImpulse2[0];
		m0->points[1].tangentImpulse = tangentImpulse2[0];
		m0->points[1].maxNormalImpulse = maxNormalImpulse2[0];
		m0->points[1].normalVelocity = normalVelocity2[0];

		m1->points[0].normalImpulse = normalImpulse1[1];
		m1->points[0].tangentImpulse = tangentImpulse1[1];
		m1->points[0].maxNormalImpulse = maxNormalImpulse1[1];
		m1->points[0].normalVelocity = normalVelocity1[1];

		m1->points[1].normalImpulse = normalImpulse2[1];
		m1->points[1].tangentImpulse = tangentImpulse2[1];
		m1->points[1].maxNormalImpulse = maxNormalImpulse2[1];
		m1->points[1].normalVelocity = normalVelocity2[1];

		m2->points[0].normalImpulse = normalImpulse1[2];
		m2->points[0].tangentImpulse = tangentImpulse1[2];
		m2->points[0].maxNormalImpulse = maxNormalImpulse1[2];
		m2->points[0].normalVelocity = normalVelocity1[2];

		m2->points[1].normalImpulse = normalImpulse2[2];
		m2->points[1].tangentImpulse = tangentImpulse2[2];
		m2->points[1].maxNormalImpulse = maxNormalImpulse2[2];
		m2->points[1].normalVelocity = normalVelocity2[2];

		m3->points[0].normalImpulse = normalImpulse1[3];
		m3->points[0].tangentImpulse = tangentImpulse1[3];
		m3->points[0].maxNormalImpulse = maxNormalImpulse1[3];
		m3->points[0].normalVelocity = normalVelocity1[3];

		m3->points[1].normalImpulse = normalImpulse2[3];
		m3->points[1].tangentImpulse = tangentImpulse2[3];
		m3->points[1].maxNormalImpulse = maxNormalImpulse2[3];
		m3->points[1].normalVelocity = normalVelocity2[3];
	}
}

#endif

#if defined( NH_SIMD_AVX2 )

// This is a load and 8x8 transpose
SimdBody Physics::GatherBodies(const BodyState* states, int* indices)
{
	// b2BodyState b2_identityBodyState = {{0.0f, 0.0f}, 0.0f, 0, {0.0f, 0.0f}, {1.0f, 0.0f}};
	FloatW identity = _mm256_setr_ps(0.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 1.0f, 0.0f);
	FloatW b0 = indices[0] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[0]));
	FloatW b1 = indices[1] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[1]));
	FloatW b2 = indices[2] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[2]));
	FloatW b3 = indices[3] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[3]));
	FloatW b4 = indices[4] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[4]));
	FloatW b5 = indices[5] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[5]));
	FloatW b6 = indices[6] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[6]));
	FloatW b7 = indices[7] == NullIndex ? identity : _mm256_load_ps((float*)(states + indices[7]));

	FloatW t0 = _mm256_unpacklo_ps(b0, b1);
	FloatW t1 = _mm256_unpackhi_ps(b0, b1);
	FloatW t2 = _mm256_unpacklo_ps(b2, b3);
	FloatW t3 = _mm256_unpackhi_ps(b2, b3);
	FloatW t4 = _mm256_unpacklo_ps(b4, b5);
	FloatW t5 = _mm256_unpackhi_ps(b4, b5);
	FloatW t6 = _mm256_unpacklo_ps(b6, b7);
	FloatW t7 = _mm256_unpackhi_ps(b6, b7);
	FloatW tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
	FloatW tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
	FloatW tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
	FloatW tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));

	SimdBody simdBody;
	simdBody.v.x = _mm256_permute2f128_ps(tt0, tt4, 0x20);
	simdBody.v.y = _mm256_permute2f128_ps(tt1, tt5, 0x20);
	simdBody.w = _mm256_permute2f128_ps(tt2, tt6, 0x20);
	simdBody.flags = _mm256_permute2f128_ps(tt3, tt7, 0x20);
	simdBody.dp.x = _mm256_permute2f128_ps(tt0, tt4, 0x31);
	simdBody.dp.y = _mm256_permute2f128_ps(tt1, tt5, 0x31);
	simdBody.dq.c = _mm256_permute2f128_ps(tt2, tt6, 0x31);
	simdBody.dq.s = _mm256_permute2f128_ps(tt3, tt7, 0x31);
	return simdBody;
}

// This writes everything back to the solver bodies but only the velocities change
void Physics::ScatterBodies(BodyState* states, int* indices, const SimdBody& simdBody)
{
	FloatW t0 = _mm256_unpacklo_ps(simdBody.v.x, simdBody.v.y);
	FloatW t1 = _mm256_unpackhi_ps(simdBody.v.x, simdBody.v.y);
	FloatW t2 = _mm256_unpacklo_ps(simdBody.w, simdBody.flags);
	FloatW t3 = _mm256_unpackhi_ps(simdBody.w, simdBody.flags);
	FloatW t4 = _mm256_unpacklo_ps(simdBody.dp.x, simdBody.dp.y);
	FloatW t5 = _mm256_unpackhi_ps(simdBody.dp.x, simdBody.dp.y);
	FloatW t6 = _mm256_unpacklo_ps(simdBody.dq.c, simdBody.dq.s);
	FloatW t7 = _mm256_unpackhi_ps(simdBody.dq.c, simdBody.dq.s);
	FloatW tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
	FloatW tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
	FloatW tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
	FloatW tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
	FloatW tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));

	// I don't use any dummy body in the body array because this will lead to multithreaded sharing and the
	// associated cache flushing.
	if (indices[0] != NullIndex) { _mm256_store_ps((float*)(states + indices[0]), _mm256_permute2f128_ps(tt0, tt4, 0x20)); }
	if (indices[1] != NullIndex) { _mm256_store_ps((float*)(states + indices[1]), _mm256_permute2f128_ps(tt1, tt5, 0x20)); }
	if (indices[2] != NullIndex) { _mm256_store_ps((float*)(states + indices[2]), _mm256_permute2f128_ps(tt2, tt6, 0x20)); }
	if (indices[3] != NullIndex) { _mm256_store_ps((float*)(states + indices[3]), _mm256_permute2f128_ps(tt3, tt7, 0x20)); }
	if (indices[4] != NullIndex) { _mm256_store_ps((float*)(states + indices[4]), _mm256_permute2f128_ps(tt0, tt4, 0x31)); }
	if (indices[5] != NullIndex) { _mm256_store_ps((float*)(states + indices[5]), _mm256_permute2f128_ps(tt1, tt5, 0x31)); }
	if (indices[6] != NullIndex) { _mm256_store_ps((float*)(states + indices[6]), _mm256_permute2f128_ps(tt2, tt6, 0x31)); }
	if (indices[7] != NullIndex) { _mm256_store_ps((float*)(states + indices[7]), _mm256_permute2f128_ps(tt3, tt7, 0x31)); }
}

#elif defined( NH_SIMD_NEON )

// This is a load and transpose
SimdBody Physics::GatherBodies(const BodyState* states, int* indices)
{
	// [vx vy w flags]
	FloatW identityA = ZeroFloatW;

	// [dpx dpy dqc dqs]

	FloatW identityB = SetW(0.0f, 0.0f, 1.0f, 0.0f);

	FloatW b1a = indices[0] == NullIndex ? identityA : LoadW((float*)(states + indices[0]) + 0);
	FloatW b1b = indices[0] == NullIndex ? identityB : LoadW((float*)(states + indices[0]) + 4);
	FloatW b2a = indices[1] == NullIndex ? identityA : LoadW((float*)(states + indices[1]) + 0);
	FloatW b2b = indices[1] == NullIndex ? identityB : LoadW((float*)(states + indices[1]) + 4);
	FloatW b3a = indices[2] == NullIndex ? identityA : LoadW((float*)(states + indices[2]) + 0);
	FloatW b3b = indices[2] == NullIndex ? identityB : LoadW((float*)(states + indices[2]) + 4);
	FloatW b4a = indices[3] == NullIndex ? identityA : LoadW((float*)(states + indices[3]) + 0);
	FloatW b4b = indices[3] == NullIndex ? identityB : LoadW((float*)(states + indices[3]) + 4);

	// [vx1 vx3 vy1 vy3]
	FloatW t1a = UnpackLoW(b1a, b3a);

	// [vx2 vx4 vy2 vy4]
	FloatW t2a = UnpackLoW(b2a, b4a);

	// [w1 w3 f1 f3]
	FloatW t3a = UnpackHiW(b1a, b3a);

	// [w2 w4 f2 f4]
	FloatW t4a = UnpackHiW(b2a, b4a);

	SimdBody simdBody;
	simdBody.v.x = UnpackLoW(t1a, t2a);
	simdBody.v.y = UnpackHiW(t1a, t2a);
	simdBody.w = UnpackLoW(t3a, t4a);
	simdBody.flags = UnpackHiW(t3a, t4a);

	FloatW t1b = UnpackLoW(b1b, b3b);
	FloatW t2b = UnpackLoW(b2b, b4b);
	FloatW t3b = UnpackHiW(b1b, b3b);
	FloatW t4b = UnpackHiW(b2b, b4b);

	simdBody.dp.x = UnpackLoW(t1b, t2b);
	simdBody.dp.y = UnpackHiW(t1b, t2b);
	simdBody.dq.c = UnpackLoW(t3b, t4b);
	simdBody.dq.s = UnpackHiW(t3b, t4b);

	return simdBody;
}

// This writes only the velocities back to the solver bodies
// https://developer.arm.com/documentation/102107a/0100/Floating-point-4x4-matrix-transposition
void Physics::ScatterBodies(BodyState* states, int* indices, const SimdBody& simdBody)
{
	// transpose
	float32x4x2_t r1 = vtrnq_f32(simdBody.v.x, simdBody.v.y);
	float32x4x2_t r2 = vtrnq_f32(simdBody.w, simdBody.flags);

	// I don't use any dummy body in the body array because this will lead to multithreaded sharing and the
	// associated cache flushing.
	if (indices[0] != NullIndex)
	{
		float32x4_t body1 = vcombine_f32(vget_low_f32(r1.val[0]), vget_low_f32(r2.val[0]));
		StoreW((float*)(states + indices[0]), body1);
	}

	if (indices[1] != NullIndex)
	{
		float32x4_t body2 = vcombine_f32(vget_low_f32(r1.val[1]), vget_low_f32(r2.val[1]));
		StoreW((float*)(states + indices[1]), body2);
	}

	if (indices[2] != NullIndex)
	{
		float32x4_t body3 = vcombine_f32(vget_high_f32(r1.val[0]), vget_high_f32(r2.val[0]));
		StoreW((float*)(states + indices[2]), body3);
	}

	if (indices[3] != NullIndex)
	{
		float32x4_t body4 = vcombine_f32(vget_high_f32(r1.val[1]), vget_high_f32(r2.val[1]));
		StoreW((float*)(states + indices[3]), body4);
	}
}

#elif defined( NH_SIMD_SSE2 )

// This is a load and transpose
SimdBody Physics::GatherBodies(const BodyState* states, int* indices)
{
	// [vx vy w flags]
	FloatW identityA = ZeroFloatW;

	// [dpx dpy dqc dqs]
	FloatW identityB = SetW(0.0f, 0.0f, 1.0f, 0.0f);

	FloatW b1a = indices[0] == NullIndex ? identityA : LoadW((float*)(states + indices[0]) + 0);
	FloatW b1b = indices[0] == NullIndex ? identityB : LoadW((float*)(states + indices[0]) + 4);
	FloatW b2a = indices[1] == NullIndex ? identityA : LoadW((float*)(states + indices[1]) + 0);
	FloatW b2b = indices[1] == NullIndex ? identityB : LoadW((float*)(states + indices[1]) + 4);
	FloatW b3a = indices[2] == NullIndex ? identityA : LoadW((float*)(states + indices[2]) + 0);
	FloatW b3b = indices[2] == NullIndex ? identityB : LoadW((float*)(states + indices[2]) + 4);
	FloatW b4a = indices[3] == NullIndex ? identityA : LoadW((float*)(states + indices[3]) + 0);
	FloatW b4b = indices[3] == NullIndex ? identityB : LoadW((float*)(states + indices[3]) + 4);

	// [vx1 vx3 vy1 vy3]
	FloatW t1a = UnpackLoW(b1a, b3a);

	// [vx2 vx4 vy2 vy4]
	FloatW t2a = UnpackLoW(b2a, b4a);

	// [w1 w3 f1 f3]
	FloatW t3a = UnpackHiW(b1a, b3a);

	// [w2 w4 f2 f4]
	FloatW t4a = UnpackHiW(b2a, b4a);

	SimdBody simdBody;
	simdBody.v.x = UnpackLoW(t1a, t2a);
	simdBody.v.y = UnpackHiW(t1a, t2a);
	simdBody.w = UnpackLoW(t3a, t4a);
	simdBody.flags = UnpackHiW(t3a, t4a);

	FloatW t1b = UnpackLoW(b1b, b3b);
	FloatW t2b = UnpackLoW(b2b, b4b);
	FloatW t3b = UnpackHiW(b1b, b3b);
	FloatW t4b = UnpackHiW(b2b, b4b);

	simdBody.dp.x = UnpackLoW(t1b, t2b);
	simdBody.dp.y = UnpackHiW(t1b, t2b);
	simdBody.dq.c = UnpackLoW(t3b, t4b);
	simdBody.dq.s = UnpackHiW(t3b, t4b);

	return simdBody;
}

// This writes only the velocities back to the solver bodies
void Physics::ScatterBodies(BodyState* states, int* indices, const SimdBody& simdBody)
{
	// [vx1 vy1 vx2 vy2]
	FloatW t1 = UnpackLoW(simdBody.v.x, simdBody.v.y);
	// [vx3 vy3 vx4 vy4]
	FloatW t2 = UnpackHiW(simdBody.v.x, simdBody.v.y);
	// [w1 f1 w2 f2]
	FloatW t3 = UnpackLoW(simdBody.w, simdBody.flags);
	// [w3 f3 w4 f4]
	FloatW t4 = UnpackHiW(simdBody.w, simdBody.flags);

	// I don't use any dummy body in the body array because this will lead to multithreaded sharing and the
	// associated cache flushing.
	if (indices[0] != NullIndex)
	{
		// [t1.x t1.y t3.x t3.y]
		StoreW((float*)(states + indices[0]), _mm_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0)));
	}

	if (indices[1] != NullIndex)
	{
		// [t1.z t1.w t3.z t3.w]
		StoreW((float*)(states + indices[1]), _mm_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2)));
	}

	if (indices[2] != NullIndex)
	{
		// [t2.x t2.y t4.x t4.y]
		StoreW((float*)(states + indices[2]), _mm_shuffle_ps(t2, t4, _MM_SHUFFLE(1, 0, 1, 0)));
	}

	if (indices[3] != NullIndex)
	{
		// [t2.z t2.w t4.z t4.w]
		StoreW((float*)(states + indices[3]), _mm_shuffle_ps(t2, t4, _MM_SHUFFLE(3, 2, 3, 2)));
	}
}

#else

// This is a load and transpose
static SimdBody GatherBodies(const BodyState* states, int* indices)
{
	BodyState identity = BodyStateIdentity;

	BodyState s1 = indices[0] == NullIndex ? identity : states[indices[0]];
	BodyState s2 = indices[1] == NullIndex ? identity : states[indices[1]];
	BodyState s3 = indices[2] == NullIndex ? identity : states[indices[2]];
	BodyState s4 = indices[3] == NullIndex ? identity : states[indices[3]];

	SimdBody simdBody;
	simdBody.v.x = (FloatW){ s1.linearVelocity.x, s2.linearVelocity.x, s3.linearVelocity.x, s4.linearVelocity.x };
	simdBody.v.y = (FloatW){ s1.linearVelocity.y, s2.linearVelocity.y, s3.linearVelocity.y, s4.linearVelocity.y };
	simdBody.w = (FloatW){ s1.angularVelocity, s2.angularVelocity, s3.angularVelocity, s4.angularVelocity };
	simdBody.flags = (FloatW){ (float)s1.flags, (float)s2.flags, (float)s3.flags, (float)s4.flags };
	simdBody.dp.x = (FloatW){ s1.deltaPosition.x, s2.deltaPosition.x, s3.deltaPosition.x, s4.deltaPosition.x };
	simdBody.dp.y = (FloatW){ s1.deltaPosition.y, s2.deltaPosition.y, s3.deltaPosition.y, s4.deltaPosition.y };
	simdBody.dq.c = (FloatW){ s1.deltaRotation.c, s2.deltaRotation.c, s3.deltaRotation.c, s4.deltaRotation.c };
	simdBody.dq.s = (FloatW){ s1.deltaRotation.s, s2.deltaRotation.s, s3.deltaRotation.s, s4.deltaRotation.s };

	return simdBody;
}

// This writes only the velocities back to the solver bodies
static void ScatterBodies(BodyState* states, int* indices, const SimdBody& simdBody)
{
	if (indices[0] != NullIndex)
	{
		BodyState& state = states[indices[0]];
		state.linearVelocity.x = simdBody.v.x.x;
		state.linearVelocity.y = simdBody.v.y.x;
		state.angularVelocity = simdBody.w.x;
	}

	if (indices[1] != NullIndex)
	{
		BodyState& state = states[indices[1]];
		state.linearVelocity.x = simdBody.v.x.y;
		state.linearVelocity.y = simdBody.v.y.y;
		state.angularVelocity = simdBody.w.y;
	}

	if (indices[2] != NullIndex)
	{
		BodyState& state = states[indices[2]];
		state.linearVelocity.x = simdBody.v.x.z;
		state.linearVelocity.y = simdBody.v.y.z;
		state.angularVelocity = simdBody.w.z;
	}

	if (indices[3] != NullIndex)
	{
		BodyState& state = states[indices[3]];
		state.linearVelocity.x = simdBody.v.x.w;
		state.linearVelocity.y = simdBody.v.y.w;
		state.angularVelocity = simdBody.w.w;
	}
}

#endif

void Physics::FinalizeBodiesTask(int startIndex, int endIndex, U32 threadIndex, StepContext& stepContext)
{
	BodyState* states = stepContext.states;
	BodySim* sims = stepContext.sims;
	float timeStep = stepContext.dt;
	float invTimeStep = stepContext.inv_dt;

	// The body move event array has should already have the correct size
	BodyMoveEvent* moveEvents = bodyMoveEvents.Data();

	Bitset& enlargedSimBitSet = taskContexts[threadIndex].enlargedSimBitset;
	Bitset& awakeIslandBitSet = taskContexts[threadIndex].awakeIslandBitset;
	TaskContext& taskContext = taskContexts[threadIndex];

	for (int simIndex = startIndex; simIndex < endIndex; ++simIndex)
	{
		BodyState& state = states[simIndex];
		BodySim& sim = sims[simIndex];

		Vector2 v = state.linearVelocity;
		float w = state.angularVelocity;

		sim.center = sim.center + state.deltaPosition;
		sim.transform.rotation = (state.deltaRotation * sim.transform.rotation).Normalized();

		// Use the velocity of the farthest point on the body to account for rotation.
		float maxVelocity = v.Magnitude() + Math::Abs(w) * sim.maxExtent;

		// Sleep needs to observe position correction as well as true velocity.
		float maxDeltaPosition = state.deltaPosition.Magnitude() + Math::Abs(state.deltaRotation.x) * sim.maxExtent;

		// Position correction is not as important for sleep as true velocity.
		float positionSleepFactor = 0.5f;

		float sleepVelocity = Math::Max(maxVelocity, positionSleepFactor * invTimeStep * maxDeltaPosition);

		// reset state deltas
		state.deltaPosition = Vector2Zero;
		state.deltaRotation = Quaternion2Identity;

		sim.transform.position = sim.center - sim.localCenter * sim.transform.rotation;

		// cache miss here, however I need the shape list below
		RigidBody2D& body = rigidBodies[sim.bodyId];
		body.bodyMoveIndex = simIndex;
		moveEvents[simIndex].transform = sim.transform;
		moveEvents[simIndex].bodyId = sim.bodyId + 1;
		moveEvents[simIndex].userData = body.userData;
		moveEvents[simIndex].fellAsleep = false;

		// reset applied force and torque
		sim.force = Vector2Zero;
		sim.torque = 0.0f;

		body.isSpeedCapped = sim.isSpeedCapped;
		sim.isSpeedCapped = false;

		sim.isFast = false;

		if (enableSleep == false || body.enableSleep == false || sleepVelocity > body.sleepThreshold)
		{
			// Body is not sleepy
			body.sleepTime = 0.0f;

			const float saftetyFactor = 0.5f;
			if (body.type == BODY_TYPE_DYNAMIC && enableContinuous && maxVelocity * timeStep > saftetyFactor * sim.minExtent)
			{
				// Store in fast array for the continuous collision stage
				// This is deterministic because the order of TOI sweeps doesn't matter
				if (sim.isBullet)
				{
					int bulletIndex = stepContext.bulletBodyCount.fetch_add(1);
					stepContext.bulletBodies[bulletIndex] = simIndex;
				}
				else
				{
					int fastIndex = stepContext.fastBodyCount.fetch_add(1);
					stepContext.fastBodies[fastIndex] = simIndex;
				}

				sim.isFast = true;
			}
			else
			{
				// Body is safe to advance
				sim.center0 = sim.center;
				sim.rotation0 = sim.transform.rotation;
			}
		}
		else
		{
			// Body is safe to advance and is falling asleep
			sim.center0 = sim.center;
			sim.rotation0 = sim.transform.rotation;
			body.sleepTime += timeStep;
		}

		// Any single body in an island can keep it awake
		Island& island = islands[body.islandId];
		if (body.sleepTime < TimeToSleep)
		{
			// keep island awake
			int islandIndex = island.localIndex;
			awakeIslandBitSet.SetBit(islandIndex);
		}
		else if (island.constraintRemoveCount > 0)
		{
			// body wants to sleep but its island needs splitting first
			if (body.sleepTime > taskContext.splitSleepTime)
			{
				// pick the sleepiest candidate
				taskContext.splitIslandId = body.islandId;
				taskContext.splitSleepTime = body.sleepTime;
			}
		}

		// Update shapes AABBs
		Transform2D transform = sim.transform;
		bool isFast = sim.isFast;
		int shapeId = body.headShapeId;
		while (shapeId != NullIndex)
		{
			Shape& shape = shapes[shapeId];

			if (isFast)
			{
				// The AABB is updated after continuous collision.
				// Add to moved shapes regardless of AABB changes.
				shape.isFast = true;

				// Bit-set to keep the move array sorted
				enlargedSimBitSet.SetBit(simIndex);
			}
			else
			{
				AABB aabb = shape.ComputeShapeAABB(transform);
				aabb.lowerBound.x -= SpeculativeDistance;
				aabb.lowerBound.y -= SpeculativeDistance;
				aabb.upperBound.x += SpeculativeDistance;
				aabb.upperBound.y += SpeculativeDistance;
				shape.aabb = aabb;

				if (shape.fatAABB.Contains(aabb) == false)
				{
					AABB fatAABB;
					fatAABB.lowerBound.x = aabb.lowerBound.x - AABBMargin;
					fatAABB.lowerBound.y = aabb.lowerBound.y - AABBMargin;
					fatAABB.upperBound.x = aabb.upperBound.x + AABBMargin;
					fatAABB.upperBound.y = aabb.upperBound.y + AABBMargin;
					shape.fatAABB = fatAABB;

					shape.enlargedAABB = true;

					// Bit-set to keep the move array sorted
					enlargedSimBitSet.SetBit(simIndex);
				}
			}

			shapeId = shape.nextShapeId;
		}
	}
}

void Physics::FastBodyTask(int startIndex, int endIndex, U32 threadIndex, StepContext& stepContext)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		int simIndex = stepContext.fastBodies[i];
		SolveContinuous(simIndex);
	}
}

void Physics::BulletBodyTask(int startIndex, int endIndex, U32 threadIndex, StepContext& stepContext)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		int simIndex = stepContext.bulletBodies[i];
		SolveContinuous(simIndex);
	}
}

void Physics::SolveContinuous(int bodySimIndex)
{
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];
	BodySim& fastBodySim = awakeSet.bodySims[bodySimIndex];

	Sweep sweep;
	sweep.Create(fastBodySim);

	Transform2D xf1;
	xf1.rotation = sweep.q1;
	xf1.position = sweep.c1 - sweep.localCenter * sweep.q1;

	Transform2D xf2;
	xf2.rotation = sweep.q2;
	xf2.position = sweep.c2 - sweep.localCenter * sweep.q2;

	DynamicTree& staticTree = Broadphase::trees[BODY_TYPE_STATIC];
	DynamicTree& kinematicTree = Broadphase::trees[BODY_TYPE_KINEMATIC];
	DynamicTree& dynamicTree = Broadphase::trees[BODY_TYPE_DYNAMIC];

	ContinuousContext context;
	context.sweep = sweep;
	context.fastBodySim = &fastBodySim;
	context.fraction = 1.0f;

	bool isBullet = fastBodySim.isBullet;

	RigidBody2D& fastBody = rigidBodies[fastBodySim.bodyId];
	int shapeId = fastBody.headShapeId;
	while (shapeId != NullIndex)
	{
		Shape& fastShape = shapes[shapeId];

		shapeId = fastShape.nextShapeId;

		// Clear flag (keep set on body)
		fastShape.isFast = false;

		context.fastShape = &fastShape;
		context.centroid1 = fastShape.localCentroid * xf1;
		context.centroid2 = fastShape.localCentroid * xf2;

		AABB box1 = fastShape.aabb;
		AABB box2 = fastShape.ComputeShapeAABB(xf2);
		AABB box = AABB::Combine(box1, box2);

		// Store this for later
		fastShape.aabb = box2;

		// No continuous collision for sensors
		if (fastShape.isSensor) { continue; }

		staticTree.QueryContinuous(box, DefaultLayerMask, context);

		if (isBullet)
		{
			kinematicTree.QueryContinuous(box, DefaultLayerMask, context);
			dynamicTree.QueryContinuous(box, DefaultLayerMask, context);
		}
	}

	const float speculativeDistance = SpeculativeDistance;
	const float aabbMargin = AABBMargin;

	if (context.fraction < 1.0f)
	{
		// Handle time of impact event
		Quaternion2 q = sweep.q1.NLerp(sweep.q2, context.fraction);
		Vector2 c = Math::Lerp(sweep.c1, sweep.c2, context.fraction);
		Vector2 origin = c - sweep.localCenter * q;

		// Advance body
		Transform2D transform = { origin, q };
		fastBodySim.transform = transform;
		fastBodySim.center = c;
		fastBodySim.rotation0 = q;
		fastBodySim.center0 = c;

		// Prepare AABBs for broad-phase
		shapeId = fastBody.headShapeId;
		while (shapeId != NullIndex)
		{
			Shape& shape = shapes[shapeId];

			// Must recompute aabb at the interpolated transform
			AABB aabb = shape.ComputeShapeAABB(transform);
			aabb.lowerBound.x -= speculativeDistance;
			aabb.lowerBound.y -= speculativeDistance;
			aabb.upperBound.x += speculativeDistance;
			aabb.upperBound.y += speculativeDistance;
			shape.aabb = aabb;

			if (shape.fatAABB.Contains(aabb) == false)
			{
				AABB fatAABB;
				fatAABB.lowerBound.x = aabb.lowerBound.x - aabbMargin;
				fatAABB.lowerBound.y = aabb.lowerBound.y - aabbMargin;
				fatAABB.upperBound.x = aabb.upperBound.x + aabbMargin;
				fatAABB.upperBound.y = aabb.upperBound.y + aabbMargin;
				shape.fatAABB = fatAABB;

				shape.enlargedAABB = true;
				fastBodySim.enlargeAABB = true;
			}

			shapeId = shape.nextShapeId;
		}
	}
	else
	{
		// No time of impact event

		// Advance body
		fastBodySim.rotation0 = fastBodySim.transform.rotation;
		fastBodySim.center0 = fastBodySim.center;

		// Prepare AABBs for broad-phase
		shapeId = fastBody.headShapeId;
		while (shapeId != NullIndex)
		{
			Shape& shape = shapes[shapeId];

			// shape->aabb is still valid

			if (shape.fatAABB.Contains(shape.aabb) == false)
			{
				AABB fatAABB;
				fatAABB.lowerBound.x = shape.aabb.lowerBound.x - aabbMargin;
				fatAABB.lowerBound.y = shape.aabb.lowerBound.y - aabbMargin;
				fatAABB.upperBound.x = shape.aabb.upperBound.x + aabbMargin;
				fatAABB.upperBound.y = shape.aabb.upperBound.y + aabbMargin;
				shape.fatAABB = fatAABB;

				shape.enlargedAABB = true;
				fastBodySim.enlargeAABB = true;
			}

			shapeId = shape.nextShapeId;
		}
	}
}

TOIOutput Physics::TimeOfImpact(const TOIInput& input)
{
	TOIOutput output;
	output.state = TOI_STATE_UNKNOWN;
	output.t = input.tMax;

	const DistanceProxy& proxyA = input.proxyA;
	const DistanceProxy& proxyB = input.proxyB;

	Sweep sweepA = input.sweepA;
	Sweep sweepB = input.sweepB;

	float tMax = input.tMax;

	float totalRadius = proxyA.radius + proxyB.radius;
	float target = Math::Max(LinearSlop, totalRadius - LinearSlop);
	float tolerance = 0.25f * LinearSlop;

	float t1 = 0.0f;
	const int k_maxIterations = 20;
	int iter = 0;

	// Prepare input for distance query.
	DistanceCache cache = { 0 };
	DistanceInput distanceInput;
	distanceInput.proxyA = input.proxyA;
	distanceInput.proxyB = input.proxyB;
	distanceInput.useRadii = false;

	// The outer loop progressively attempts to compute new separating axes.
	// This loop terminates when an axis is repeated (no progress is made).
	while (true)
	{
		Transform2D xfA = GetSweepTransform(sweepA, t1);
		Transform2D xfB = GetSweepTransform(sweepB, t1);

		// Get the distance between shapes. We can also use the results
		// to get a separating axis.
		distanceInput.transformA = xfA;
		distanceInput.transformB = xfB;
		DistanceOutput distanceOutput = ShapeDistance(cache, distanceInput, nullptr, 0);

		// If the shapes are overlapped, we give up on continuous collision.
		if (distanceOutput.distance <= 0.0f)
		{
			// Failure!
			output.state = TOI_STATE_OVERLAPPED;
			output.t = 0.0f;
			break;
		}

		if (distanceOutput.distance < target + tolerance)
		{
			// Victory!
			output.state = TOI_STATE_HIT;
			output.t = t1;
			break;
		}

		// Initialize the separating axis.
		SeparationFunction fcn = MakeSeparationFunction(cache, proxyA, sweepA, proxyB, sweepB, t1);

		// Compute the TOI on the separating axis. We do this by successively
		// resolving the deepest point. This loop is bounded by the number of vertices.
		bool done = false;
		float t2 = tMax;
		int pushBackIter = 0;
		for (;; )
		{
			// Find the deepest point at t2. Store the witness point indices.
			int indexA, indexB;
			float s2 = FindMinSeparation(fcn, indexA, indexB, t2);

			// Is the final configuration separated?
			if (s2 > target + tolerance)
			{
				// Victory!
				output.state = TOI_STATE_SEPARATED;
				output.t = tMax;
				done = true;
				break;
			}

			// Has the separation reached tolerance?
			if (s2 > target - tolerance)
			{
				// Advance the sweeps
				t1 = t2;
				break;
			}

			// Compute the initial separation of the witness points.
			float s1 = EvaluateSeparation(fcn, indexA, indexB, t1);

			// Check for initial overlap. This might happen if the root finder
			// runs out of iterations.
			if (s1 < target - tolerance)
			{
				output.state = TOI_STATE_FAILED;
				output.t = t1;
				done = true;
				break;
			}

			// Check for touching
			if (s1 <= target + tolerance)
			{
				// Victory! t1 should hold the TOI (could be 0.0).
				output.state = TOI_STATE_HIT;
				output.t = t1;
				done = true;
				break;
			}

			// Compute 1D root of: f(x) - target = 0
			int rootIterCount = 0;
			float a1 = t1, a2 = t2;
			for (;; )
			{
				// Use a mix of the secant rule and bisection.
				float t;
				if (rootIterCount & 1)
				{
					// Secant rule to improve convergence.
					t = a1 + (target - s1) * (a2 - a1) / (s2 - s1);
				}
				else
				{
					// Bisection to guarantee progress.
					t = 0.5f * (a1 + a2);
				}

				++rootIterCount;

				float s = EvaluateSeparation(fcn, indexA, indexB, t);

				if (Math::Abs(s - target) < tolerance)
				{
					// t2 holds a tentative value for t1
					t2 = t;
					break;
				}

				// Ensure we continue to bracket the root.
				if (s > target)
				{
					a1 = t;
					s1 = s;
				}
				else
				{
					a2 = t;
					s2 = s;
				}

				if (rootIterCount == 50) { break; }
			}

			++pushBackIter;

			if (pushBackIter == MaxPolygonVertices) { break; }
		}

		++iter;

		if (done) { break; }

		if (iter == k_maxIterations)
		{
			// Root finder got stuck. Semi-victory.
			output.state = TOI_STATE_FAILED;
			output.t = t1;
			break;
		}
	}

	return output;
}

SeparationFunction Physics::MakeSeparationFunction(const DistanceCache& cache, const DistanceProxy& proxyA, const Sweep& sweepA,
	const DistanceProxy& proxyB, const Sweep& sweepB, F32 t1)
{
	SeparationFunction f;

	f.proxyA = &proxyA;
	f.proxyB = &proxyB;
	int count = cache.count;

	f.sweepA = sweepA;
	f.sweepB = sweepB;

	Transform2D xfA = GetSweepTransform(sweepA, t1);
	Transform2D xfB = GetSweepTransform(sweepB, t1);

	if (count == 1)
	{
		f.type = SEPARATION_TYPE_POINTS;
		Vector2 localPointA = proxyA.points[cache.indexA[0]];
		Vector2 localPointB = proxyB.points[cache.indexB[0]];
		Vector2 pointA = localPointA * xfA;
		Vector2 pointB = localPointB * xfB;
		f.axis = (pointB - pointA).Normalized();
		f.localPoint = Vector2Zero;
		return f;
	}

	if (cache.indexA[0] == cache.indexA[1])
	{
		// Two points on B and one on A.
		f.type = SEPARATION_TYPE_FACE_B;
		Vector2 localPointB1 = proxyB.points[cache.indexB[0]];
		Vector2 localPointB2 = proxyB.points[cache.indexB[1]];

		f.axis = (localPointB2 - localPointB1).Cross(1.0f);
		f.axis = f.axis.Normalized();
		Vector2 normal = f.axis * xfB.rotation;

		f.localPoint = { 0.5f * (localPointB1.x + localPointB2.x), 0.5f * (localPointB1.y + localPointB2.y) };
		Vector2 pointB = f.localPoint * xfB;

		Vector2 localPointA = proxyA.points[cache.indexA[0]];
		Vector2 pointA = localPointA * xfA;

		float s = (pointA - pointB).Dot(normal);
		if (s < 0.0f) { f.axis = -f.axis; }
		return f;
	}

	// Two points on A and one or two points on B.
	f.type = SEPARATION_TYPE_FACE_A;
	Vector2 localPointA1 = proxyA.points[cache.indexA[0]];
	Vector2 localPointA2 = proxyA.points[cache.indexA[1]];

	f.axis = localPointA2 - localPointA1.Cross(1.0f);
	f.axis = f.axis.Normalized();
	Vector2 normal = f.axis * xfA.rotation;

	f.localPoint = { 0.5f * (localPointA1.x + localPointA2.x), 0.5f * (localPointA1.y + localPointA2.y) };
	Vector2 pointA = f.localPoint * xfA;

	Vector2 localPointB = proxyB.points[cache.indexB[0]];
	Vector2 pointB = localPointB * xfB;

	float s = (pointB - pointA).Dot(normal);
	if (s < 0.0f)
	{
		f.axis = -f.axis;
	}
	return f;
}

F32 Physics::FindMinSeparation(const SeparationFunction& f, int& indexA, int& indexB, float t)
{
	Transform2D xfA = GetSweepTransform(f.sweepA, t);
	Transform2D xfB = GetSweepTransform(f.sweepB, t);

	switch (f.type)
	{
	case SEPARATION_TYPE_POINTS:
	{
		Vector2 axisA = f.axis ^ xfA.rotation;
		Vector2 axisB = -f.axis ^ xfB.rotation;

		indexA = FindSupport(*f.proxyA, axisA);
		indexB = FindSupport(*f.proxyB, axisB);

		Vector2 localPointA = f.proxyA->points[indexA];
		Vector2 localPointB = f.proxyB->points[indexB];

		Vector2 pointA = localPointA * xfA;
		Vector2 pointB = localPointB * xfB;

		float separation = (pointB - pointA).Dot(f.axis);
		return separation;
	}

	case SEPARATION_TYPE_FACE_A:
	{
		Vector2 normal = f.axis * xfA.rotation;
		Vector2 pointA = f.localPoint * xfA;

		Vector2 axisB = -normal ^ xfB.rotation;

		indexA = -1;
		indexB = FindSupport(*f.proxyB, axisB);

		Vector2 localPointB = f.proxyB->points[indexB];
		Vector2 pointB = localPointB * xfB;

		float separation = (pointB - pointA).Dot(normal);
		return separation;
	}

	case SEPARATION_TYPE_FACE_B:
	{
		Vector2 normal = f.axis * xfB.rotation;
		Vector2 pointB = f.localPoint * xfB;

		Vector2 axisA = -normal ^ xfA.rotation;

		indexB = -1;
		indexA = FindSupport(*f.proxyA, axisA);

		Vector2 localPointA = f.proxyA->points[indexA];
		Vector2 pointA = localPointA * xfA;

		float separation = (pointA - pointB).Dot(normal);
		return separation;
	}

	default:
		indexA = -1;
		indexB = -1;
		return 0.0f;
	}
}

float Physics::EvaluateSeparation(const SeparationFunction& f, int indexA, int indexB, float t)
{
	Transform2D xfA = GetSweepTransform(f.sweepA, t);
	Transform2D xfB = GetSweepTransform(f.sweepB, t);

	switch (f.type)
	{
	case SEPARATION_TYPE_POINTS:
	{
		Vector2 localPointA = f.proxyA->points[indexA];
		Vector2 localPointB = f.proxyB->points[indexB];

		Vector2 pointA = localPointA * xfA;
		Vector2 pointB = localPointB * xfB;

		float separation = (pointB - pointA).Dot(f.axis);
		return separation;
	}

	case SEPARATION_TYPE_FACE_A:
	{
		Vector2 normal = f.axis * xfA.rotation;
		Vector2 pointA = f.localPoint * xfA;

		Vector2 localPointB = f.proxyB->points[indexB];
		Vector2 pointB = localPointB * xfB;

		float separation = (pointB - pointA).Dot(normal);
		return separation;
	}

	case SEPARATION_TYPE_FACE_B:
	{
		Vector2 normal = f.axis * xfB.rotation;
		Vector2 pointB = f.localPoint * xfB;

		Vector2 localPointA = f.proxyA->points[indexA];
		Vector2 pointA = localPointA * xfA;

		float separation = (pointA - pointB).Dot(normal);
		return separation;
	}

	default: return 0.0f;
	}
}

Transform2D Physics::GetSweepTransform(const Sweep& sweep, F32 time)
{
	Transform2D xf;
	xf.position = Math::Lerp(sweep.c1, sweep.c2, time);
	xf.rotation = Math::Lerp(sweep.q1, sweep.q2, time).Normalized();
	xf.position -= sweep.localCenter * xf.rotation;
	return xf;
}

Island& Physics::CreateIsland(int setIndex)
{
	int islandId = islandFreelist.GetFree();

	if (islandId == islands.Size()) { islands.Push({}); }

	SolverSet& set = solverSets[setIndex];

	Island& island = islands[islandId];
	island.setIndex = setIndex;
	island.localIndex = (I32)set.islandSims.Size();
	island.islandId = islandId;
	island.headBody = NullIndex;
	island.tailBody = NullIndex;
	island.bodyCount = 0;
	island.headContact = NullIndex;
	island.tailContact = NullIndex;
	island.contactCount = 0;
	island.headJoint = NullIndex;
	island.tailJoint = NullIndex;
	island.jointCount = 0;
	island.parentIsland = NullIndex;
	island.constraintRemoveCount = 0;

	IslandSim& islandSim = set.islandSims.Push({});
	islandSim.islandId = islandId;

	return island;
}

void Physics::DestroyIsland(int islandId)
{
	// assume island is empty
	Island& island = islands[islandId];
	SolverSet& set = solverSets[island.setIndex];
	int movedIndex = set.islandSims.RemoveSwap(island.localIndex);
	if (movedIndex != NullIndex)
	{
		// Fix index on moved element
		IslandSim& movedElement = set.islandSims[island.localIndex];
		int movedId = movedElement.islandId;
		Island& movedIsland = islands[movedId];
		movedIsland.localIndex = island.localIndex;
	}

	// Free island and id (preserve island revision)
	island.islandId = NullIndex;
	island.setIndex = NullIndex;
	island.localIndex = NullIndex;
	islandFreelist.Release(islandId);
}

void Physics::AddContactToIsland(I32 islandId, Contact& contact)
{
	Island& island = islands[islandId];

	if (island.headContact != NullIndex)
	{
		contact.islandNext = island.headContact;
		Contact& headContact = contacts[island.headContact];
		headContact.islandPrev = contact.contactId;
	}

	island.headContact = contact.contactId;
	if (island.tailContact == NullIndex)
	{
		island.tailContact = island.headContact;
	}

	island.contactCount += 1;
	contact.islandId = islandId;
}

void Physics::TrySleepIsland(int islandId)
{
	Island& island = islands[islandId];

	// cannot put an island to sleep while it has a pending split
	if (island.constraintRemoveCount > 0) { return; }

	// island is sleeping
	// - create new sleeping solver set
	// - move island to sleeping solver set
	// - identify non-touching contacts that should move to sleeping solver set or disabled set
	// - remove old island
	// - fix island
	int sleepSetId = solverSetFreelist.GetFree();
	if (sleepSetId == solverSets.Size())
	{
		SolverSet set = { 0 };
		set.setIndex = NullIndex;
		solverSets.Push(set);
	}

	SolverSet& sleepSet = solverSets[sleepSetId];

	// grab awake set after creating the sleep set because the solver set array may have been resized
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];

	sleepSet.setIndex = sleepSetId;
	sleepSet.bodySims.Resize(island.bodyCount);
	sleepSet.contactSims.Resize(island.contactCount);
	sleepSet.jointSims.Resize(island.jointCount);

	// move awake bodies to sleeping set
	// this shuffles around bodies in the awake set
	{
		SolverSet& disabledSet = solverSets[SET_TYPE_DISABLED];
		int bodyId = island.headBody;
		while (bodyId != NullIndex)
		{
			RigidBody2D& body = rigidBodies[bodyId];

			// Update the body move event to indicate this body fell asleep
			// It could happen the body is forced asleep before it ever moves.
			if (body.bodyMoveIndex != NullIndex)
			{
				BodyMoveEvent& moveEvent = bodyMoveEvents[body.bodyMoveIndex];
				moveEvent.fellAsleep = true;
				body.bodyMoveIndex = NullIndex;
			}

			int awakeBodyIndex = body.localIndex;
			BodySim& awakeSim = awakeSet.bodySims[awakeBodyIndex];

			// move body sim to sleep set
			int sleepBodyIndex = (I32)sleepSet.bodySims.Size();
			sleepSet.bodySims.Push(awakeSim);

			int movedIndex = awakeSet.bodySims.RemoveSwap(awakeBodyIndex);
			if (movedIndex != NullIndex)
			{
				// fix local index on moved element
				BodySim& movedSim = awakeSet.bodySims[awakeBodyIndex];
				int movedId = movedSim.bodyId;
				RigidBody2D& movedBody = rigidBodies[movedId];
				movedBody.localIndex = awakeBodyIndex;
			}

			// destroy state, no need to clone
			awakeSet.bodyStates.RemoveSwap(awakeBodyIndex);

			body.setIndex = sleepSetId;
			body.localIndex = sleepBodyIndex;

			// Move non-touching contacts to the disabled set.
			// Non-touching contacts may exist between sleeping islands and there is no clear ownership.
			int contactKey = body.headContactKey;
			while (contactKey != NullIndex)
			{
				int contactId = contactKey >> 1;
				int edgeIndex = contactKey & 1;

				Contact& contact = contacts[contactId];

				contactKey = contact.edges[edgeIndex].nextKey;

				if (contact.setIndex == SET_TYPE_DISABLED) { continue; }
				if (contact.colorIndex != NullIndex) { continue; }

				// the other body may still be awake, it still may go to sleep and then it will be responsible
				// for moving this contact to the disabled set.
				int otherEdgeIndex = edgeIndex ^ 1;
				int otherBodyId = contact.edges[otherEdgeIndex].bodyId;
				RigidBody2D& otherBody = rigidBodies[otherBodyId];
				if (otherBody.setIndex == SET_TYPE_AWAKE) { continue; }

				int localIndex = contact.localIndex;
				ContactSim& contactSim = awakeSet.contactSims[localIndex];

				// move the non-touching contact to the disabled set
				contact.setIndex = SET_TYPE_DISABLED;
				contact.localIndex = (I32)disabledSet.contactSims.Size();
				disabledSet.contactSims.Push(contactSim);

				int movedLocalIndex = awakeSet.contactSims.RemoveSwap(localIndex);
				if (movedLocalIndex != NullIndex)
				{
					// fix moved element
					ContactSim& movedContactSim = awakeSet.contactSims[localIndex];
					Contact& movedContact = contacts[movedContactSim.contactId];
					movedContact.localIndex = localIndex;
				}
			}

			bodyId = body.islandNext;
		}
	}

	// move touching contacts
	// this shuffles contacts in the awake set
	{
		int contactId = island.headContact;
		while (contactId != NullIndex)
		{
			Contact& contact = contacts[contactId];
			int colorIndex = contact.colorIndex;

			GraphColor& color = constraintGraph.colors[colorIndex];

			// Remove bodies from graph coloring associated with this constraint
			if (colorIndex != OverflowIndex)
			{
				// might clear a bit for a static body, but this has no effect
				color.bodySet.ClearBit(contact.edges[0].bodyId);
				color.bodySet.ClearBit(contact.edges[1].bodyId);
			}

			int localIndex = contact.localIndex;
			ContactSim& awakeContactSim = color.contactSims[localIndex];

			int sleepContactIndex = (I32)sleepSet.contactSims.Size();
			sleepSet.contactSims.Push(awakeContactSim);

			int movedLocalIndex = color.contactSims.RemoveSwap(localIndex);
			if (movedLocalIndex != NullIndex)
			{
				// fix moved element
				ContactSim& movedContactSim = color.contactSims[localIndex];
				Contact& movedContact = contacts[movedContactSim.contactId];
				movedContact.localIndex = localIndex;
			}

			contact.setIndex = sleepSetId;
			contact.colorIndex = NullIndex;
			contact.localIndex = sleepContactIndex;

			contactId = contact.islandNext;
		}
	}

	// move joints
	// this shuffles joints in the awake set
	{
		int jointId = island.headJoint;
		while (jointId != NullIndex)
		{
			Joint& joint = joints[jointId];
			int colorIndex = joint.colorIndex;
			int localIndex = joint.localIndex;

			GraphColor& color = constraintGraph.colors[colorIndex];

			JointSim& awakeJointSim = color.jointSims[localIndex];

			if (colorIndex != OverflowIndex)
			{
				// might clear a bit for a static body, but this has no effect
				color.bodySet.ClearBit(joint.edges[0].bodyId);
				color.bodySet.ClearBit(joint.edges[1].bodyId);
			}

			int sleepJointIndex = (I32)sleepSet.jointSims.Size();
			sleepSet.jointSims.Push(awakeJointSim);

			int movedIndex = color.jointSims.RemoveSwap(localIndex);
			if (movedIndex != NullIndex)
			{
				// fix moved element
				JointSim& movedJointSim = color.jointSims[localIndex];
				int movedId = movedJointSim.jointId;
				Joint& movedJoint = joints[movedId];
				movedJoint.localIndex = localIndex;
			}

			joint.setIndex = sleepSetId;
			joint.colorIndex = NullIndex;
			joint.localIndex = sleepJointIndex;

			jointId = joint.islandNext;
		}
	}

	// move island struct
	{
		int islandIndex = island.localIndex;
		IslandSim& sleepIsland = sleepSet.islandSims.Push({});
		sleepIsland.islandId = islandId;

		int movedIslandIndex = awakeSet.islandSims.RemoveSwap(islandIndex);
		if (movedIslandIndex != NullIndex)
		{
			// fix index on moved element
			IslandSim& movedIslandSim = awakeSet.islandSims[islandIndex];
			int movedIslandId = movedIslandSim.islandId;
			Island& movedIsland = islands[movedIslandId];
			movedIsland.localIndex = islandIndex;
		}

		island.setIndex = sleepSetId;
		island.localIndex = 0;
	}
}

void Physics::SplitIsland(int baseId)
{
	Island& baseIsland = islands[baseId];
	int setIndex = baseIsland.setIndex;

	if (setIndex != SET_TYPE_AWAKE) { return; }

	if (baseIsland.constraintRemoveCount == 0) { return; }

	int bodyCount = baseIsland.bodyCount;

	// No lock is needed because I ensure the allocator is not used while this task is active.
	int* stack;
	Memory::AllocateArray(&stack, bodyCount);
	int* bodyIds;
	Memory::AllocateArray(&bodyIds, bodyCount);

	// Build array containing all body indices from base island. These
	// serve as seed bodies for the depth first search (DFS).
	int index = 0;
	int nextBody = baseIsland.headBody;
	while (nextBody != NullIndex)
	{
		bodyIds[index++] = nextBody;
		RigidBody2D& body = rigidBodies[nextBody];

		// Clear visitation mark
		body.isMarked = false;

		nextBody = body.islandNext;
	}

	// Clear contact island flags. Only need to consider contacts
	// already in the base island.
	int nextContactId = baseIsland.headContact;
	while (nextContactId != NullIndex)
	{
		Contact& contact = contacts[nextContactId];
		contact.isMarked = false;
		nextContactId = contact.islandNext;
	}

	// Clear joint island flags.
	int nextJoint = baseIsland.headJoint;
	while (nextJoint != NullIndex)
	{
		Joint& joint = joints[nextJoint];
		joint.isMarked = false;
		nextJoint = joint.islandNext;
	}

	// Done with the base split island.
	DestroyIsland(baseId);

	// Each island is found as a depth first search starting from a seed body
	for (int i = 0; i < bodyCount; ++i)
	{
		int seedIndex = bodyIds[i];
		RigidBody2D& seed = rigidBodies[seedIndex];

		if (seed.isMarked == true)
		{
			// The body has already been visited
			continue;
		}

		int stackCount = 0;
		stack[stackCount++] = seedIndex;
		seed.isMarked = true;

		// Create new island
		// No lock needed because only a single island can split per time step. No islands are being used during the constraint
		// solve. However, islands are touched during body finalization.
		Island& island = CreateIsland(setIndex);

		int islandId = island.islandId;

		// Perform a depth first search (DFS) on the constraint graph.
		while (stackCount > 0)
		{
			// Grab the next body off the stack and add it to the island.
			int bodyId = stack[--stackCount];
			RigidBody2D& body = rigidBodies[bodyId];

			// Add body to island
			body.islandId = islandId;
			if (island.tailBody != NullIndex)
			{
				rigidBodies[island.tailBody].islandNext = bodyId;
			}
			body.islandPrev = island.tailBody;
			body.islandNext = NullIndex;
			island.tailBody = bodyId;

			if (island.headBody == NullIndex)
			{
				island.headBody = bodyId;
			}

			island.bodyCount += 1;

			// Search all contacts connected to this body.
			int contactKey = body.headContactKey;
			while (contactKey != NullIndex)
			{
				int contactId = contactKey >> 1;
				int edgeIndex = contactKey & 1;

				Contact& contact = contacts[contactId];

				// Next key
				contactKey = contact.edges[edgeIndex].nextKey;

				// Has this contact already been added to this island?
				if (contact.isMarked) { continue; }

				// Skip sensors
				if (contact.flags & CONTACT_FLAG_SENSOR) { continue; }

				// Is this contact enabled and touching?
				if ((contact.flags & CONTACT_FLAG_TOUCHING) == 0) { continue; }

				contact.isMarked = true;

				int otherEdgeIndex = edgeIndex ^ 1;
				int otherBodyId = contact.edges[otherEdgeIndex].bodyId;
				RigidBody2D& otherBody = rigidBodies[otherBodyId];

				// Maybe add other body to stack
				if (otherBody.isMarked == false && otherBody.setIndex != SET_TYPE_STATIC)
				{
					stack[stackCount++] = otherBodyId;
					otherBody.isMarked = true;
				}

				// Add contact to island
				contact.islandId = islandId;
				if (island.tailContact != NullIndex)
				{
					Contact& tailContact = contacts[island.tailContact];
					tailContact.islandNext = contactId;
				}
				contact.islandPrev = island.tailContact;
				contact.islandNext = NullIndex;
				island.tailContact = contactId;

				if (island.headContact == NullIndex)
				{
					island.headContact = contactId;
				}

				island.contactCount += 1;
			}

			// Search all joints connect to this body.
			int jointKey = body.headJointKey;
			while (jointKey != NullIndex)
			{
				int jointId = jointKey >> 1;
				int edgeIndex = jointKey & 1;

				Joint& joint = joints[jointId];

				// Next key
				jointKey = joint.edges[edgeIndex].nextKey;

				// Has this joint already been added to this island?
				if (joint.isMarked) { continue; }

				joint.isMarked = true;

				int otherEdgeIndex = edgeIndex ^ 1;
				int otherBodyId = joint.edges[otherEdgeIndex].bodyId;
				RigidBody2D& otherBody = rigidBodies[otherBodyId];

				// Don't simulate joints connected to disabled bodies.
				if (otherBody.setIndex == SET_TYPE_DISABLED) { continue; }

				// Maybe add other body to stack
				if (otherBody.isMarked == false && otherBody.setIndex == SET_TYPE_AWAKE)
				{
					stack[stackCount++] = otherBodyId;
					otherBody.isMarked = true;
				}

				// Add joint to island
				joint.islandId = islandId;
				if (island.tailJoint != NullIndex)
				{
					Joint& tailJoint = joints[island.tailJoint];
					tailJoint.islandNext = jointId;
				}
				joint.islandPrev = island.tailJoint;
				joint.islandNext = NullIndex;
				island.tailJoint = jointId;

				if (island.headJoint == NullIndex)
				{
					island.headJoint = jointId;
				}

				island.jointCount += 1;
			}
		}
	}

	Memory::Free(&bodyIds);
	Memory::Free(&stack);
}

void Physics::CreateIslandForBody(int setIndex, RigidBody2D& body)
{
	Island& island = CreateIsland(setIndex);

	body.islandId = island.islandId;
	island.headBody = body.id;
	island.tailBody = body.id;
	island.bodyCount = 1;
}

void Physics::RemoveBodyFromIsland(RigidBody2D& body)
{
	if (body.islandId == NullIndex) { return; }

	int islandId = body.islandId;
	Island& island = islands[islandId];

	// Fix the island's linked list of sims
	if (body.islandPrev != NullIndex)
	{
		RigidBody2D& prevBody = rigidBodies[body.islandPrev];
		prevBody.islandNext = body.islandNext;
	}

	if (body.islandNext != NullIndex)
	{
		RigidBody2D& nextBody = rigidBodies[body.islandNext];
		nextBody.islandPrev = body.islandPrev;
	}

	island.bodyCount -= 1;
	bool islandDestroyed = false;

	if (island.headBody == body.id)
	{
		island.headBody = body.islandNext;

		if (island.headBody == NullIndex)
		{
			// Free the island
			DestroyIsland(island.islandId);
			islandDestroyed = true;
		}
	}
	else if (island.tailBody == body.id)
	{
		island.tailBody = body.islandPrev;
	}

	body.islandId = NullIndex;
	body.islandPrev = NullIndex;
	body.islandNext = NullIndex;
}

I32 Physics::GetBodyID(RigidBody2D& body)
{
	return (I32)rigidBodies.Index(&body);
}

bool Physics::WakeBody(RigidBody2D& body)
{
	if (body.setIndex >= SET_TYPE_FIRST_SLEEPING)
	{
		WakeSolverSet(body.setIndex);
		return true;
	}

	return false;
}

bool Physics::ShouldBodiesCollide(const RigidBody2D& bodyA, const RigidBody2D& bodyB)
{
	if (bodyA.type != BODY_TYPE_DYNAMIC && bodyB.type != BODY_TYPE_DYNAMIC) { return false; }

	I32 jointKey;
	I32 otherBodyId;
	if (bodyA.jointCount < bodyB.jointCount)
	{
		jointKey = bodyA.headJointKey;
		otherBodyId = bodyB.id;
	}
	else
	{
		jointKey = bodyB.headJointKey;
		otherBodyId = bodyA.id;
	}

	while (jointKey != NullIndex)
	{
		I32 jointId = jointKey >> 1;
		I32 edgeIndex = jointKey & 1;
		I32 otherEdgeIndex = edgeIndex ^ 1;

		Joint& joint = joints[jointId];
		if (!joint.collideConnected && joint.edges[otherEdgeIndex].bodyId == otherBodyId) { return false; }

		jointKey = joint.edges[edgeIndex].nextKey;
	}

	return true;
}

bool Physics::TestShapeOverlap(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache)
{
	DistanceInput input;
	input.proxyA = MakeShapeDistanceProxy(shapeA);
	input.proxyB = MakeShapeDistanceProxy(shapeB);
	input.transformA = xfA;
	input.transformB = xfB;
	input.useRadii = true;

	DistanceOutput output = ShapeDistance(cache, input, nullptr, 0);

	return output.distance < 10.0f * Traits<F32>::Epsilon;
}

DistanceProxy Physics::MakeShapeDistanceProxy(const Shape& shape)
{
	switch (shape.type)
	{
	case SHAPE_TYPE_CAPSULE: return MakeProxy(&shape.capsule.center1, 2, shape.capsule.radius);
	case SHAPE_TYPE_CIRCLE: return MakeProxy(&shape.circle.center, 1, shape.circle.radius);
	case SHAPE_TYPE_POLYGON: return MakeProxy(shape.polygon.vertices, shape.polygon.count, shape.polygon.radius);
	case SHAPE_TYPE_SEGMENT: return MakeProxy(&shape.segment.point1, 2, 0.0f);
	case SHAPE_TYPE_CHAIN_SEGMENT: return MakeProxy(&shape.chainSegment.segment.point1, 2, 0.0f);
	default: return {};
	}
}

// GJK using Voronoi regions (Christer Ericson) and Barycentric coordinates.
// todo try not copying
DistanceProxy Physics::MakeProxy(const Vector2* vertices, I32 count, F32 radius)
{
	count = Math::Min(count, (I32)MaxPolygonVertices);
	DistanceProxy proxy;
	for (int i = 0; i < count; ++i) { proxy.points[i] = vertices[i]; }
	proxy.count = count;
	proxy.radius = radius;
	return proxy;
}

DistanceOutput Physics::ShapeDistance(DistanceCache& cache, const DistanceInput& input, Simplex* simplexes, I32 simplexCapacity)
{
	DistanceOutput output = {};

	const DistanceProxy& proxyA = input.proxyA;
	const DistanceProxy& proxyB = input.proxyB;

	Transform2D transformA = input.transformA;
	Transform2D transformB = input.transformB;

	// Initialize the simplex.
	Simplex simplex = MakeSimplexFromCache(cache, proxyA, transformA, proxyB, transformB);

	I32 simplexIndex = 0;
	if (simplexes != NULL && simplexIndex < simplexCapacity)
	{
		simplexes[simplexIndex] = simplex;
		simplexIndex += 1;
	}

	// Get simplex vertices as an array.
	SimplexVertex* vertices[] = { &simplex.v1, &simplex.v2, &simplex.v3 };
	const I32 k_maxIters = 20;

	// These store the vertices of the last simplex so that we can check for duplicates and prevent cycling.
	I32 saveA[3], saveB[3];

	// Main iteration loop.
	I32 iter = 0;
	while (iter < k_maxIters)
	{
		// Copy simplex so we can identify duplicates.
		I32 saveCount = simplex.count;
		for (I32 i = 0; i < saveCount; ++i)
		{
			saveA[i] = vertices[i]->indexA;
			saveB[i] = vertices[i]->indexB;
		}

		switch (simplex.count)
		{
		case 1: break;

		case 2:
			SolveSimplex2(simplex);
			break;

		case 3:
			SolveSimplex3(simplex);
			break;
		}

		// If we have 3 points, then the origin is in the corresponding triangle.
		if (simplex.count == 3) { break; }

		if (simplexes != NULL && simplexIndex < simplexCapacity)
		{
			simplexes[simplexIndex] = simplex;
			simplexIndex += 1;
		}

		// Get search direction.
		Vector2 d = ComputeSimplexSearchDirection(simplex);

		// Ensure the search direction is numerically fit.
		if (d.Dot(d) < Traits<F32>::Epsilon * Traits<F32>::Epsilon)
		{
			// The origin is probably contained by a line segment
			// or triangle. Thus the shapes are overlapped.

			// We can't return zero here even though there may be overlap.
			// In case the simplex is a point, segment, or triangle it is difficult
			// to determine if the origin is contained in the CSO or very close to it.
			break;
		}

		// Compute a tentative new simplex vertex using support points.
		// support = support(b, d) - support(a, -d)
		SimplexVertex* vertex = vertices[simplex.count];
		vertex->indexA = FindSupport(proxyA, -d ^ transformA.rotation);
		vertex->wA = proxyA.points[vertex->indexA] * transformA;
		vertex->indexB = FindSupport(proxyB, d ^ transformB.rotation);
		vertex->wB = proxyB.points[vertex->indexB] * transformB;
		vertex->w = vertex->wB - vertex->wA;

		// Iteration count is equated to the number of support point calls.
		++iter;

		// Check for duplicate support points. This is the main termination criteria.
		bool duplicate = false;
		for (int i = 0; i < saveCount; ++i)
		{
			if (vertex->indexA == saveA[i] && vertex->indexB == saveB[i])
			{
				duplicate = true;
				break;
			}
		}

		// If we found a duplicate support point we must exit to avoid cycling.
		if (duplicate) { break; }

		// New vertex is ok and needed.
		++simplex.count;
	}

	if (simplexes != NULL && simplexIndex < simplexCapacity)
	{
		simplexes[simplexIndex] = simplex;
		simplexIndex += 1;
	}

	// Prepare output
	ComputeSimplexWitnessPoints(output.pointA, output.pointB, simplex);
	output.distance = (output.pointA - output.pointB).Magnitude();
	output.iterations = iter;
	output.simplexCount = simplexIndex;

	// Cache the simplex
	MakeSimplexCache(cache, simplex);

	// Apply radii if requested
	if (input.useRadii)
	{
		if (output.distance < Traits<F32>::Epsilon)
		{
			// Shapes are too close to safely compute normal
			Vector2 p = { 0.5f * (output.pointA.x + output.pointB.x), 0.5f * (output.pointA.y + output.pointB.y) };
			output.pointA = p;
			output.pointB = p;
			output.distance = 0.0f;
		}
		else
		{
			// Keep closest points on perimeter even if overlapped, this way
			// the points move smoothly.
			F32 rA = proxyA.radius;
			F32 rB = proxyB.radius;
			output.distance = Math::Max(0.0f, output.distance - rA - rB);
			Vector2 normal = (output.pointB - output.pointA).Normalized();
			Vector2 offsetA = { rA * normal.x, rA * normal.y };
			Vector2 offsetB = { rB * normal.x, rB * normal.y };
			output.pointA = output.pointA + offsetA;
			output.pointB = output.pointB - offsetB;
		}
	}

	return output;
}

Simplex Physics::MakeSimplexFromCache(const DistanceCache& cache, const DistanceProxy& proxyA, const Transform2D& transformA, const DistanceProxy& proxyB, const Transform2D& transformB)
{
	Simplex s;

	// Copy data from cache.
	s.count = cache.count;

	SimplexVertex* vertices[] = { &s.v1, &s.v2, &s.v3 };
	for (int i = 0; i < s.count; ++i)
	{
		SimplexVertex* v = vertices[i];
		v->indexA = cache.indexA[i];
		v->indexB = cache.indexB[i];
		Vector2 wALocal = proxyA.points[v->indexA];
		Vector2 wBLocal = proxyB.points[v->indexB];
		v->wA = wALocal * transformA;
		v->wB = wBLocal * transformB;
		v->w = v->wB - v->wA;

		// invalid
		v->a = -1.0f;
	}

	// If the cache is empty or invalid ...
	if (s.count == 0)
	{
		SimplexVertex* v = vertices[0];
		v->indexA = 0;
		v->indexB = 0;
		Vector2 wALocal = proxyA.points[0];
		Vector2 wBLocal = proxyB.points[0];
		v->wA = wALocal * transformA;
		v->wB = wBLocal * transformB;
		v->w = v->wB - v->wA;
		v->a = 1.0f;
		s.count = 1;
	}

	return s;
}

void Physics::MakeSimplexCache(DistanceCache& cache, const Simplex& simplex)
{
	cache.count = (U16)simplex.count;
	const SimplexVertex* vertices[] = { &simplex.v1, &simplex.v2, &simplex.v3 };
	for (int i = 0; i < simplex.count; ++i)
	{
		cache.indexA[i] = (U8)vertices[i]->indexA;
		cache.indexB[i] = (U8)vertices[i]->indexB;
	}
}

void Physics::SolveSimplex2(Simplex& s)
{
	Vector2 w1 = s.v1.w;
	Vector2 w2 = s.v2.w;
	Vector2 e12 = w2 - w1;

	// w1 region
	F32 d12_2 = -w1.Dot(e12);
	if (d12_2 <= 0.0f)
	{
		// a2 <= 0, so we clamp it to 0
		s.v1.a = 1.0f;
		s.count = 1;
		return;
	}

	// w2 region
	F32 d12_1 = w2.Dot(e12);
	if (d12_1 <= 0.0f)
	{
		// a1 <= 0, so we clamp it to 0
		s.v2.a = 1.0f;
		s.count = 1;
		s.v1 = s.v2;
		return;
	}

	// Must be in e12 region.
	F32 inv_d12 = 1.0f / (d12_1 + d12_2);
	s.v1.a = d12_1 * inv_d12;
	s.v2.a = d12_2 * inv_d12;
	s.count = 2;
}

void Physics::SolveSimplex3(Simplex& s)
{
	Vector2 w1 = s.v1.w;
	Vector2 w2 = s.v2.w;
	Vector2 w3 = s.v3.w;

	// Edge12
	// [1      1     ][a1] = [1]
	// [w1.e12 w2.e12][a2] = [0]
	// a3 = 0
	Vector2 e12 = w2 - w1;
	F32 w1e12 = w1.Dot(e12);
	F32 w2e12 = w2.Dot(e12);
	F32 d12_1 = w2e12;
	F32 d12_2 = -w1e12;

	// Edge13
	// [1      1     ][a1] = [1]
	// [w1.e13 w3.e13][a3] = [0]
	// a2 = 0
	Vector2 e13 = w3 - w1;
	F32 w1e13 = w1.Dot(e13);
	F32 w3e13 = w3.Dot(e13);
	F32 d13_1 = w3e13;
	F32 d13_2 = -w1e13;

	// Edge23
	// [1      1     ][a2] = [1]
	// [w2.e23 w3.e23][a3] = [0]
	// a1 = 0
	Vector2 e23 = w3 - w2;
	F32 w2e23 = w2.Dot(e23);
	F32 w3e23 = w3.Dot(e23);
	F32 d23_1 = w3e23;
	F32 d23_2 = -w2e23;

	// Triangle123
	F32 n123 = e12.Cross(e13);

	F32 d123_1 = n123 * w2.Cross(w3);
	F32 d123_2 = n123 * w3.Cross(w1);
	F32 d123_3 = n123 * w1.Cross(w2);

	// w1 region
	if (d12_2 <= 0.0f && d13_2 <= 0.0f)
	{
		s.v1.a = 1.0f;
		s.count = 1;
		return;
	}

	// e12
	if (d12_1 > 0.0f && d12_2 > 0.0f && d123_3 <= 0.0f)
	{
		F32 inv_d12 = 1.0f / (d12_1 + d12_2);
		s.v1.a = d12_1 * inv_d12;
		s.v2.a = d12_2 * inv_d12;
		s.count = 2;
		return;
	}

	// e13
	if (d13_1 > 0.0f && d13_2 > 0.0f && d123_2 <= 0.0f)
	{
		F32 inv_d13 = 1.0f / (d13_1 + d13_2);
		s.v1.a = d13_1 * inv_d13;
		s.v3.a = d13_2 * inv_d13;
		s.count = 2;
		s.v2 = s.v3;
		return;
	}

	// w2 region
	if (d12_1 <= 0.0f && d23_2 <= 0.0f)
	{
		s.v2.a = 1.0f;
		s.count = 1;
		s.v1 = s.v2;
		return;
	}

	// w3 region
	if (d13_1 <= 0.0f && d23_1 <= 0.0f)
	{
		s.v3.a = 1.0f;
		s.count = 1;
		s.v1 = s.v3;
		return;
	}

	// e23
	if (d23_1 > 0.0f && d23_2 > 0.0f && d123_1 <= 0.0f)
	{
		F32 inv_d23 = 1.0f / (d23_1 + d23_2);
		s.v2.a = d23_1 * inv_d23;
		s.v3.a = d23_2 * inv_d23;
		s.count = 2;
		s.v1 = s.v3;
		return;
	}

	// Must be in triangle123
	F32 inv_d123 = 1.0f / (d123_1 + d123_2 + d123_3);
	s.v1.a = d123_1 * inv_d123;
	s.v2.a = d123_2 * inv_d123;
	s.v3.a = d123_3 * inv_d123;
	s.count = 3;
}

Vector2 Physics::ComputeSimplexSearchDirection(const Simplex& simplex)
{
	switch (simplex.count)
	{
	case 1:
		return -simplex.v1.w;

	case 2:
	{
		Vector2 e12 = simplex.v2.w - simplex.v1.w;
		F32 sgn = e12.Cross(-simplex.v1.w);
		if (sgn > 0.0f) { return e12.PerpendicularLeft(); }
		else { return e12.PerpendicularRight(); }
	}

	default: return Vector2Zero;
	}
}

I32 Physics::FindSupport(const DistanceProxy& proxy, const Vector2& direction)
{
	I32 bestIndex = 0;
	F32 bestValue = proxy.points[0].Dot(direction);
	for (I32 i = 1; i < proxy.count; ++i)
	{
		F32 value = proxy.points[i].Dot(direction);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return bestIndex;
}

Vector2 Physics::Weight2(F32 a1, const Vector2& w1, F32 a2, const Vector2& w2)
{
	return { a1 * w1.x + a2 * w2.x, a1 * w1.y + a2 * w2.y };
}

Vector2 Physics::Weight3(F32 a1, const Vector2& w1, F32 a2, const Vector2& w2, F32 a3, const Vector2& w3)
{
	return { a1 * w1.x + a2 * w2.x + a3 * w3.x, a1 * w1.y + a2 * w2.y + a3 * w3.y };
}

void Physics::MergeAwakeIslands()
{
	SolverSet& awakeSet = solverSets[SET_TYPE_AWAKE];

	// Step 1: Ensure every child island points to its root island. This avoids merging a child island with
	// a parent island that has already been merged with a grand-parent island.
	for (int i = 0; i < awakeSet.islandSims.Size(); ++i)
	{
		int islandId = awakeSet.islandSims[i].islandId;

		Island* island = &islands[islandId];

		// find the root island
		int rootId = islandId;
		Island* rootIsland = island;
		while (rootIsland->parentIsland != NullIndex)
		{
			Island* parent = &islands[rootIsland->parentIsland];
			if (parent->parentIsland != NullIndex)
			{
				// path compression
				rootIsland->parentIsland = parent->parentIsland;
			}

			rootId = rootIsland->parentIsland;
			rootIsland = parent;
		}

		if (rootIsland != island)
		{
			island->parentIsland = rootId;
		}
	}

	// Step 2: merge every awake island into its parent (which must be a root island)
	// Reverse to support removal from awake array.
	for (int i = (I32)awakeSet.islandSims.Size() - 1; i >= 0; --i)
	{
		int islandId = awakeSet.islandSims[i].islandId;
		Island& island = islands[islandId];

		if (island.parentIsland == NullIndex) { continue; }

		MergeIsland(island);

		// this call does a remove swap from the end of the island sim array
		DestroyIsland(islandId);
	}
}

void Physics::MergeIsland(Island& island)
{
	int rootId = island.parentIsland;
	Island& rootIsland = islands[rootId];

	// remap island indices
	int bodyId = island.headBody;
	while (bodyId != NullIndex)
	{
		RigidBody2D& body = rigidBodies[bodyId];
		body.islandId = rootId;
		bodyId = body.islandNext;
	}

	int contactId = island.headContact;
	while (contactId != NullIndex)
	{
		Contact& contact = contacts[contactId];
		contact.islandId = rootId;
		contactId = contact.islandNext;
	}

	int jointId = island.headJoint;
	while (jointId != NullIndex)
	{
		Joint& joint = joints[jointId];
		joint.islandId = rootId;
		jointId = joint.islandNext;
	}

	// connect body lists
	RigidBody2D& tailBody = rigidBodies[rootIsland.tailBody];
	tailBody.islandNext = island.headBody;

	RigidBody2D& headBody = rigidBodies[island.headBody];
	headBody.islandPrev = rootIsland.tailBody;

	rootIsland.tailBody = island.tailBody;
	rootIsland.bodyCount += island.bodyCount;

	// connect contact lists
	if (rootIsland.headContact == NullIndex)
	{
		// Root island has no contacts
		rootIsland.headContact = island.headContact;
		rootIsland.tailContact = island.tailContact;
		rootIsland.contactCount = island.contactCount;
	}
	else if (island.headContact != NullIndex)
	{
		// Both islands have contacts
		Contact& tailContact = contacts[rootIsland.tailContact];
		tailContact.islandNext = island.headContact;

		Contact& headContact = contacts[island.headContact];
		headContact.islandPrev = rootIsland.tailContact;

		rootIsland.tailContact = island.tailContact;
		rootIsland.contactCount += island.contactCount;
	}

	if (rootIsland.headJoint == NullIndex)
	{
		// Root island has no joints
		rootIsland.headJoint = island.headJoint;
		rootIsland.tailJoint = island.tailJoint;
		rootIsland.jointCount = island.jointCount;
	}
	else if (island.headJoint != NullIndex)
	{
		// Both islands have joints
		Joint& tailJoint = joints[rootIsland.tailJoint];
		tailJoint.islandNext = island.headJoint;

		Joint& headJoint = joints[island.headJoint];
		headJoint.islandPrev = rootIsland.tailJoint;

		rootIsland.tailJoint = island.tailJoint;
		rootIsland.jointCount += island.jointCount;
	}

	// Track removed constraints
	rootIsland.constraintRemoveCount += island.constraintRemoveCount;
}

void Physics::ComputeSimplexWitnessPoints(Vector2& a, Vector2& b, const Simplex& s)
{
	switch (s.count)
	{
	case 0: break;

	case 1:
		a = s.v1.wA;
		b = s.v1.wB;
		break;

	case 2:
		a = Weight2(s.v1.a, s.v1.wA, s.v2.a, s.v2.wA);
		b = Weight2(s.v1.a, s.v1.wB, s.v2.a, s.v2.wB);
		break;

	case 3:
		a = Weight3(s.v1.a, s.v1.wA, s.v2.a, s.v2.wA, s.v3.a, s.v3.wA);
		// TODO_ERIN why are these not equal?
		//*b = b2Weight3(s->v1.a, s->v1.wB, s->v2.a, s->v2.wB, s->v3.a, s->v3.wB);
		b = a;
		break;

	default: break;
	}
}
