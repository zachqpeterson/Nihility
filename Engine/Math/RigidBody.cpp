#include "RigidBody.hpp"

#include "Physics.hpp"
#include "Broadphase.hpp"
#include "Resources\Scene.hpp"

import Memory;
import Math;

RigidBody2D::RigidBody2D(const RigidBody2DDef& def)
{
	bool isAwake = (def.isAwake || def.enableSleep == false) && def.isEnabled;

	// determine the solver set
	int setId;
	if (def.isEnabled == false) { setId = SET_TYPE_DISABLED; }
	else if (def.type == SET_TYPE_STATIC) { setId = SET_TYPE_STATIC; }
	else if (isAwake == true) { setId = SET_TYPE_AWAKE; }
	else
	{
		// new set for a sleeping body in its own island
		setId = Physics::solverSetFreelist.GetFree();
		if (setId == Physics::solverSets.Size())
		{
			// Create a zero initialized solver set. All sub-arrays are also zero initialized.
			Physics::solverSets.Push({});
		}

		Physics::solverSets[setId].setIndex = setId;
	}

	I32 bodyId = Physics::GetBodyID(*this);

	SolverSet& set = Physics::solverSets[setId];
	BodySim& bodySim = set.bodySims.Push({});
	bodySim.transform.position = def.position;
	bodySim.transform.rotation = def.rotation;
	bodySim.center = def.position;
	bodySim.rotation0 = bodySim.transform.rotation;
	bodySim.center0 = bodySim.center;
	bodySim.localCenter = Vector2Zero;
	bodySim.force = Vector2Zero;
	bodySim.torque = 0.0f;
	bodySim.mass = 0.0f;
	bodySim.invMass = 0.0f;
	bodySim.inertia = 0.0f;
	bodySim.invInertia = 0.0f;
	bodySim.minExtent = Huge;
	bodySim.maxExtent = 0.0f;
	bodySim.linearDamping = def.linearDamping;
	bodySim.angularDamping = def.angularDamping;
	bodySim.gravityScale = def.gravityScale;
	bodySim.bodyId = bodyId;
	bodySim.isBullet = def.isBullet;
	bodySim.allowFastRotation = def.allowFastRotation;
	bodySim.enlargeAABB = false;
	bodySim.isFast = false;
	bodySim.isSpeedCapped = false;

	if (setId == SET_TYPE_AWAKE)
	{
		BodyState& bodyState = set.bodyStates.Push({});

		bodyState.linearVelocity = def.linearVelocity;
		bodyState.angularVelocity = def.angularVelocity;
		bodyState.deltaRotation = Quaternion2Identity;
	}

	userData = def.userData;
	setIndex = setId;
	localIndex = (I32)(set.bodySims.Size() - 1);
	revision += 1;
	headShapeId = NullIndex;
	shapeCount = 0;
	headChainId = NullIndex;
	headContactKey = NullIndex;
	contactCount = 0;
	headJointKey = NullIndex;
	jointCount = 0;
	islandId = NullIndex;
	islandPrev = NullIndex;
	islandNext = NullIndex;
	bodyMoveIndex = NullIndex;
	id = bodyId;
	sleepThreshold = def.sleepThreshold;
	sleepTime = 0.0f;
	type = def.type;
	enableSleep = def.enableSleep;
	fixedRotation = def.fixedRotation;
	isSpeedCapped = false;
	isMarked = false;
	automaticMass = def.automaticMass;

	// dynamic and kinematic bodies that are enabled need a island
	if (setId >= SET_TYPE_AWAKE)
	{
		Physics::CreateIslandForBody(setId, *this);
	}
}

RigidBody2D::RigidBody2D(const RigidBody2D& other) : userData(other.userData),
	setIndex(other.setIndex),
	localIndex(other.localIndex),
	headContactKey(other.headContactKey),
	contactCount(other.contactCount),
	headShapeId(other.headShapeId),
	shapeCount(other.shapeCount),
	headChainId(other.headChainId),
	headJointKey(other.headJointKey),
	jointCount(other.jointCount),
	islandId(other.islandId),
	islandPrev(other.islandPrev),
	islandNext(other.islandNext),
	sleepThreshold(other.sleepThreshold),
	sleepTime(other.sleepTime),
	bodyMoveIndex(other.bodyMoveIndex),
	id(other.id),
	type(other.type),
	revision(other.revision),
	enableSleep(other.enableSleep),
	fixedRotation(other.fixedRotation),
	isSpeedCapped(other.isSpeedCapped),
	isMarked(other.isMarked),
	automaticMass(other.automaticMass)
{

}

RigidBody2D::RigidBody2D(RigidBody2D&& other) : userData(other.userData),
	setIndex(other.setIndex),
	localIndex(other.localIndex),
	headContactKey(other.headContactKey),
	contactCount(other.contactCount),
	headShapeId(other.headShapeId),
	shapeCount(other.shapeCount),
	headChainId(other.headChainId),
	headJointKey(other.headJointKey),
	jointCount(other.jointCount),
	islandId(other.islandId),
	islandPrev(other.islandPrev),
	islandNext(other.islandNext),
	sleepThreshold(other.sleepThreshold),
	sleepTime(other.sleepTime),
	bodyMoveIndex(other.bodyMoveIndex),
	id(other.id),
	type(other.type),
	revision(other.revision),
	enableSleep(other.enableSleep),
	fixedRotation(other.fixedRotation),
	isSpeedCapped(other.isSpeedCapped),
	isMarked(other.isMarked),
	automaticMass(other.automaticMass)
{

}

void RigidBody2D::Update(Scene* scene)
{
	BodySim& sim = Physics::GetBodySim(*this);
	Entity* e = scene->GetEntity(entityID);
	e->transform.SetRotation(sim.transform.rotation);
	e->transform.SetPosition(sim.transform.position);
}

void RigidBody2D::Load(Scene* scene)
{

}

void RigidBody2D::Cleanup(Scene* scene)
{
	bool wakeBodies = true;

	// Destroy the attached joints
	int edgeKey = headJointKey;
	while (edgeKey != NullIndex)
	{
		int jointId = edgeKey >> 1;
		int edgeIndex = edgeKey & 1;

		Joint& joint = Physics::joints[jointId];
		edgeKey = joint.edges[edgeIndex].nextKey;

		// Careful because this modifies the list being traversed
		Physics::DestroyJointInternal(joint, wakeBodies);
	}

	// Destroy all contacts attached to this body.
	DestroyBodyContacts(wakeBodies);

	// Destroy the attached shapes and their broad-phase proxies.
	int shapeId = headShapeId;
	while (shapeId != NullIndex)
	{
		Shape& shape = Physics::shapes[shapeId];

		Broadphase::DestroyProxy(shape.proxyKey);

		// Return shape to free list.
		Physics::shapeFreelist.Release(shapeId);
		shape.id = NullIndex;

		shapeId = shape.nextShapeId;
	}

	// Destroy the attached chains. The associated shapes have already been destroyed above.
	int chainId = headChainId;
	while (chainId != NullIndex)
	{
		ChainShape& chain = Physics::chains[chainId];

		Memory::Free(&chain.shapeIndices);
		chain.shapeIndices = NULL;

		// Return chain to free list.
		Physics::chainFreelist.Release(chainId);
		chain.id = NullIndex;

		chainId = chain.nextChainId;
	}

	Physics::RemoveBodyFromIsland(*this);

	// Remove body sim from solver set that owns it
	SolverSet& set = Physics::solverSets[setIndex];
	int movedIndex = set.bodySims.RemoveSwap(localIndex);
	if (movedIndex != NullIndex)
	{
		// Fix moved body index
		BodySim& movedSim = set.bodySims[localIndex];
		int movedId = movedSim.bodyId;
		RigidBody2D& movedBody = Physics::GetRigidBody(movedId);
		movedBody.localIndex = localIndex;
	}

	// Remove body state from awake set
	if (setIndex == SET_TYPE_AWAKE)
	{
		set.bodyStates.RemoveSwap(localIndex);
	}

	setIndex = NullIndex;
	localIndex = NullIndex;
	id = NullIndex;
}

void RigidBody2D::DestroyBodyContacts(bool wakeBodies)
{
	// Destroy the attached contacts
	int edgeKey = headContactKey;
	while (edgeKey != NullIndex)
	{
		int contactId = edgeKey >> 1;
		int edgeIndex = edgeKey & 1;

		Contact& contact = Physics::contacts[contactId];
		edgeKey = contact.edges[edgeIndex].nextKey;
		Physics::DestroyContact(contact, wakeBodies);
	}
}

I32 RigidBody2D::AddCollider(const ShapeDef& shapeDef, const Capsule& capsule)
{
	Transform2D& transform = Physics::solverSets[setIndex].bodySims[localIndex].transform;

	Shape& shape = Physics::CreateCapsuleShape(*this, transform, shapeDef, capsule);

	if (automaticMass) { UpdateMassData(); }

	return shape.id + 1;
}

I32 RigidBody2D::AddCollider(const ShapeDef& shapeDef, const Circle& circle)
{
	Transform2D& transform = Physics::solverSets[setIndex].bodySims[localIndex].transform;

	Shape& shape = Physics::CreateCircleShape(*this, transform, shapeDef, circle);

	if (automaticMass) { UpdateMassData(); }

	return shape.id + 1;
}

I32 RigidBody2D::AddCollider(const ShapeDef& shapeDef, const ConvexPolygon& polygon)
{
	Transform2D& transform = Physics::solverSets[setIndex].bodySims[localIndex].transform;

	Shape& shape = Physics::CreateConvexPolygonShape(*this, transform, shapeDef, polygon);

	if (automaticMass) { UpdateMassData(); }

	return shape.id + 1;
}

I32 RigidBody2D::AddCollider(const ShapeDef& shapeDef, const Segment& segment)
{
	Transform2D& transform = Physics::solverSets[setIndex].bodySims[localIndex].transform;

	Shape& shape = Physics::CreateSegmentShape(*this, transform, shapeDef, segment);

	if (automaticMass) { UpdateMassData(); }

	return shape.id + 1;
}

I32 RigidBody2D::AddCollider(const ShapeDef& shapeDef, const ChainSegment& chainSegment)
{
	Transform2D& transform = Physics::solverSets[setIndex].bodySims[localIndex].transform;

	Shape& shape = Physics::CreateChainSegmentShape(*this, transform, shapeDef, chainSegment);

	if (automaticMass) { UpdateMassData(); }

	return shape.id + 1;
}

void RigidBody2D::SetPosition(const Vector2& position)
{

}

void RigidBody2D::SetRotation(const Quaternion2& rotation)
{

}

void RigidBody2D::UpdateMassData()
{
	BodySim& bodySim = Physics::solverSets[setIndex].bodySims[localIndex];

	// Compute mass data from shapes. Each shape has its own density.
	bodySim.mass = 0.0f;
	bodySim.invMass = 0.0f;
	bodySim.inertia = 0.0f;
	bodySim.invInertia = 0.0f;
	bodySim.localCenter = Vector2Zero;
	bodySim.minExtent = Huge;
	bodySim.maxExtent = 0.0f;

	// Static and kinematic sims have zero mass.
	if (type != BODY_TYPE_DYNAMIC)
	{
		bodySim.center = bodySim.transform.position;

		// Need extents for kinematic bodies for sleeping to work correctly.
		if (type == BODY_TYPE_KINEMATIC)
		{
			int shapeId = headShapeId;
			while (shapeId != NullIndex)
			{
				const Shape& s = Physics::shapes[shapeId];

				ShapeExtent extent = Physics::ComputeShapeExtent(s, Vector2Zero);
				bodySim.minExtent = Math::Min(bodySim.minExtent, extent.minExtent);
				bodySim.maxExtent = Math::Max(bodySim.maxExtent, extent.maxExtent);

				shapeId = s.nextShapeId;
			}
		}

		return;
	}
}