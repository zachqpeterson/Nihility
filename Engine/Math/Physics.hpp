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

#pragma once

#include "PhysicsDefines.hpp"
#include "PhysicsEvents.hpp"

#include "Containers\Vector.hpp"
#include "Containers\Freelist.hpp"

struct Scene;

class NH_API Physics
{
public:
	static DistanceOutput ShapeDistance(DistanceCache& cache, const DistanceInput& input, Simplex* simplexes, I32 simplexCapacity);

	static ConvexPolygon CreatePolygon(const Hull& hull, F32 radius);
	static ConvexPolygon CreateOffsetPolygon(const Hull& hull, F32 radius, const Transform2D& transform);
	static ConvexPolygon CreateBox(F32 hx, F32 hy);
	static ConvexPolygon CreateRoundedBox(F32 hx, F32 hy, F32 radius);
	static ConvexPolygon CreateOffsetBox(F32 hx, F32 hy, const Transform2D& transform);

	static Vector2 ComputePolygonCentroid(const Vector2* vertices, I32 count);
	static MassData ComputeCircleMass(const Circle& shape, F32 density);
	static MassData ComputeCapsuleMass(const Capsule& shape, F32 density);
	static MassData ComputePolygonMass(const ConvexPolygon& shape, F32 density);
	static AABB ComputeCircleAABB(const Circle& shape, const Transform2D& transform);
	static AABB ComputeCapsuleAABB(const Capsule& shape, const Transform2D& transform);
	static ShapeExtent ComputeShapeExtent(const Shape& shape, Vector2 localCenter);

	static RigidBody2D& GetRigidBody(I32 id);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update(F32 step);
	static void Solve(StepContext& stepContext);

	static void EnableSleeping(bool flag);
	static void EnableWarmStarting(bool flag);
	static void EnableContinuous(bool flag);

	static void WakeSolverSet(int setIndex);

	static void Step(F32 timeStep, int subStepCount);
	static void Collide(StepContext& context);
	static void CollideTask(int startIndex, int endIndex, int threadIndex, StepContext& stepContext);

	static void CreateShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, Shape& shape);
	static Shape& CreateCircleShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const Circle& geometry);
	static Shape& CreateCapsuleShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const Capsule& geometry);
	static Shape& CreateConvexPolygonShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const ConvexPolygon& geometry);
	static Shape& CreateSegmentShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const Segment& geometry);
	static Shape& CreateChainSegmentShape(RigidBody2D& body, const Transform2D& transform, const ShapeDef& def, const ChainSegment& geometry);
	static BodySim& GetBodySim(RigidBody2D& body);
	
	static void PrepareOverflowContacts(StepContext& context);
	static void PrepareContactsTask(int startIndex, int endIndex, StepContext& context);
	static void WarmStartContactsTask(int startIndex, int endIndex, StepContext& context, int colorIndex);
	static void SolveContactsTask(int startIndex, int endIndex, StepContext& context, int colorIndex, bool useBias);

	static void CreateContact(Shape& shapeA, Shape& shapeB);
	static void DestroyContact(Contact& contact, bool wakeBodies);
	static bool UpdateContact(ContactSim& contactSim, Shape& shapeA, const Transform2D& transformA, const Vector2& centerOffsetA,
		Shape& shapeB, const Transform2D& transformB, const Vector2& centerOffsetB);
	static void LinkContact(Contact& contact);
	static void UnlinkContact(Contact& contact);

	static void DestroyJointInternal(Joint& joint, bool wakeBodies);
	static void UnlinkJoint(Joint& joint);
	static void RemoveJointFromGraph(int bodyIdA, int bodyIdB, int colorIndex, int localIndex);
	static bool WakeBody(RigidBody2D& body);

	static void PrepareOverflowJoints(StepContext& context);
	static void PrepareJointsTask(int startIndex, int endIndex, StepContext& context);
	static void PrepareJoint(JointSim& joint, StepContext& context);
	static void PrepareDistanceJoint(JointSim& base, StepContext& context);
	static void PrepareMotorJoint(JointSim& base, StepContext& context);
	static void PrepareMouseJoint(JointSim& base, StepContext& context);
	static void PreparePrismaticJoint(JointSim& base, StepContext& context);
	static void PrepareRevoluteJoint(JointSim& base, StepContext& context);
	static void PrepareWeldJoint(JointSim& base, StepContext& context);
	static void PrepareWheelJoint(JointSim& base, StepContext& context);

	static void WarmStartOverflowJoints(StepContext& context);
	static void WarmStartJointsTask(int startIndex, int endIndex, StepContext& context, int colorIndex);
	static void WarmStartJoint(JointSim& joint, StepContext& context);
	static void WarmStartDistanceJoint(JointSim& base, StepContext& context);
	static void WarmStartMotorJoint(JointSim& base, StepContext& context);
	static void WarmStartMouseJoint(JointSim& base, StepContext& context);
	static void WarmStartPrismaticJoint(JointSim& base, StepContext& context);
	static void WarmStartRevoluteJoint(JointSim& base, StepContext& context);
	static void WarmStartWeldJoint(JointSim& base, StepContext& context);
	static void WarmStartWheelJoint(JointSim& base, StepContext& context);

	static void SolveOverflowJoints(StepContext& context, bool useBias);
	static void SolveJointsTask(int startIndex, int endIndex, StepContext& context, int colorIndex, bool useBias);
	static void SolveJoint(JointSim& joint, StepContext& context, bool useBias);
	static void SolveDistanceJoint(JointSim& joint, StepContext& context, bool useBias);
	static void SolveMotorJoint(JointSim& joint, StepContext& context, bool useBias);
	static void SolveMouseJoint(JointSim& joint, StepContext& context);
	static void SolvePrismaticJoint(JointSim& joint, StepContext& context, bool useBias);
	static void SolveRevoluteJoint(JointSim& joint, StepContext& context, bool useBias);
	static void SolveWeldJoint(JointSim& joint, StepContext& context, bool useBias);
	static void SolveWheelJoint(JointSim& joint, StepContext& context, bool useBias);

	static void IntegrateVelocitiesTask(int startIndex, int endIndex, StepContext& context);
	static void IntegratePositionsTask(int startIndex, int endIndex, StepContext& context);
	static void ApplyRestitutionTask(int startIndex, int endIndex, StepContext& context, int colorIndex);
	static void StoreImpulsesTask(int startIndex, int endIndex, StepContext& context);

	static SimdBody GatherBodies(const BodyState* states, int* indices);
	static void ScatterBodies(BodyState* states, int* indices, const SimdBody& simdBody);

	static void RemoveNonTouchingContact(int setIndex, int localIndex);
	static void AddNonTouchingContact(Contact& contact, ContactSim& contactSim);

	static bool ShouldBodiesCollide(const RigidBody2D& bodyA, const RigidBody2D& bodyB);
	static bool TestShapeOverlap(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);

	static DistanceProxy MakeShapeDistanceProxy(const Shape& shape);
	static DistanceProxy MakeProxy(const Vector2* vertices, I32 count, F32 radius);
	static Simplex MakeSimplexFromCache(const DistanceCache& cache, const DistanceProxy& proxyA, const Transform2D& transformA, const DistanceProxy& proxyB, const Transform2D& transformB);
	static void MakeSimplexCache(DistanceCache& cache, const Simplex& simplex);
	static void SolveSimplex2(Simplex& s);
	static void SolveSimplex3(Simplex& s);
	static Vector2 ComputeSimplexSearchDirection(const Simplex& simplex);
	static I32 FindSupport(const DistanceProxy& proxy, const Vector2& direction);
	static void ComputeSimplexWitnessPoints(Vector2& a, Vector2& b, const Simplex& s);
	static Vector2 Weight2(F32 a1, const Vector2& w1, F32 a2, const Vector2& w2);
	static Vector2 Weight3(F32 a1, const Vector2& w1, F32 a2, const Vector2& w2, F32 a3, const Vector2& w3);

	static Island& CreateIsland(int setIndex);
	static void DestroyIsland(int islandId);
	static void AddContactToIsland(I32 islandId, Contact& contact);
	static void MergeAwakeIslands();
	static void MergeIsland(Island& island);
	static void TrySleepIsland(int islandId);
	static void SplitIsland(int baseId);
	static void CreateIslandForBody(int setIndex, RigidBody2D& body);
	static void RemoveBodyFromIsland(RigidBody2D& body);
	static I32 GetBodyID(RigidBody2D& body);

	static void SolverTask(WorkerContext& workerContext);
	static void ExecuteStage(SolverStage& stage, StepContext* context, int previousSyncIndex, int syncIndex, int workerIndex);
	static void ExecuteMainStage(SolverStage& stage, StepContext* context, U32 syncBits);
	static void ExecuteBlock(SolverStage& stage, StepContext& context, SolverBlock& block);
	static int GetWorkerStartIndex(int workerIndex, int blockCount, int workerCount);
	static void WarmStartOverflowContacts(StepContext& context);
	static void SolveOverflowContacts(StepContext& context, bool useBias);
	static void ApplyOverflowRestitution(StepContext& context);
	static void StoreOverflowImpulses(StepContext& context);

	static void FinalizeBodiesTask(int startIndex, int endIndex, U32 threadIndex, StepContext& stepContext);
	static void FastBodyTask(int startIndex, int endIndex, U32 threadIndex, StepContext& taskContext);
	static void BulletBodyTask(int startIndex, int endIndex, U32 threadIndex, StepContext& taskContext);
	static void SolveContinuous(int bodySimIndex);
	static TOIOutput TimeOfImpact(const TOIInput& input);
	static SeparationFunction MakeSeparationFunction(const DistanceCache& cache, const DistanceProxy& proxyA, const Sweep& sweepA,
		const DistanceProxy& proxyB, const Sweep& sweepB, F32 t1);
	static F32 FindMinSeparation(const SeparationFunction& f, int& indexA, int& indexB, float t);
	static float EvaluateSeparation(const SeparationFunction& f, int indexA, int indexB, float t);
	static Transform2D GetSweepTransform(const Sweep& sweep, F32 time);

	static ConstraintGraph constraintGraph;

	static Freelist rigidBodyFreelist;
	static Vector<RigidBody2D> rigidBodies;

	static Freelist solverSetFreelist;
	static Vector<SolverSet> solverSets;

	static Freelist jointFreelist;
	static Vector<Joint> joints;

	static Freelist contactFreelist;
	static Vector<Contact> contacts;

	static Freelist islandFreelist;
	static Vector<Island> islands;

	static Freelist shapeFreelist;
	static Vector<Shape> shapes;

	static Freelist chainFreelist;
	static Vector<ChainShape> chains;

	static Vector<TaskContext> taskContexts;

	static Vector<BodyMoveEvent> bodyMoveEvents;
	static Vector<SensorBeginTouchEvent> sensorBeginEvents;
	static Vector<SensorEndTouchEvent> sensorEndEvents;
	static Vector<ContactBeginTouchEvent> contactBeginEvents;
	static Vector<ContactEndTouchEvent> contactEndEvents;
	static Vector<ContactHitEvent> contactHitEvents;

	static U64 stepIndex;

	// Identify islands for splitting as follows:
	// - I want to split islands so smaller islands can sleep
	// - when a body comes to rest and its sleep timer trips, I can look at the island and flag it for splitting
	//   if it has removed constraints
	// - islands that have removed constraints must be put split first because I don't want to wake bodies incorrectly
	// - otherwise I can use the awake islands that have bodies wanting to sleep as the splitting candidates
	// - if no bodies want to sleep then there is no reason to perform island splitting
	static I32 splitIslandId;

	static Vector2 gravity;
	static F32 hitEventThreshold;
	static F32 restitutionThreshold;
	static F32 maxLinearVelocity;
	static F32 contactPushoutVelocity;
	static F32 contactHertz;
	static F32 contactDampingRatio;
	static F32 jointHertz;
	static F32 jointDampingRatio;

	static U16 revision;

	static I32 workerCount;
	static void* userTreeTask;

	// Remember type step used for reporting forces and torques
	static F32 inv_h;

	static I32 activeTaskCount;
	static I32 taskCount;

	static bool enableSleep;
	static bool locked;
	static bool enableWarmStarting;
	static bool enableContinuous;
	static bool paused;
	static bool singleStep;
	static int subStepCount;

	STATIC_CLASS(Physics);
	friend class Engine;
	friend class Broadphase;
	friend struct ConstraintGraph;
	friend struct Scene;
	friend struct RigidBody2D;
};