#include "PhysicsDefines.hpp"

#include "Physics.hpp"

void ConstraintGraph::Create(I32 bodyCapacity)
{
	bodyCapacity = Math::Max(bodyCapacity, 8);

	for (int i = 0; i < OverflowIndex; ++i)
	{
		GraphColor* color = colors + i;
		color->bodySet.Create(bodyCapacity);
		color->bodySet.SetBitCountAndClear(bodyCapacity);
	}
}

void ConstraintGraph::Destroy()
{
	for (I32 i = 0; i < GraphColorCount; ++i)
	{
		GraphColor* color = colors + i;
		color->bodySet.Destroy();
		color->contactSims.Destroy();
		color->jointSims.Destroy();
	}
}

void ConstraintGraph::AddContact(ContactSim& contactSim, Contact& contact)
{
	int colorIndex = OverflowIndex;

	int bodyIdA = contact.edges[0].bodyId;
	int bodyIdB = contact.edges[1].bodyId;
	RigidBody2D& bodyA = Physics::bodies->Get(bodyIdA);
	RigidBody2D& bodyB = Physics::bodies->Get(bodyIdB);
	bool staticA = bodyA.setIndex == SET_TYPE_STATIC;
	bool staticB = bodyB.setIndex == SET_TYPE_STATIC;

	if (staticA == false && staticB == false)
	{
		for (int i = 0; i < OverflowIndex; ++i)
		{
			GraphColor& color = Physics::constraintGraph.colors[i];
			if (color.bodySet.GetBit(bodyIdA) || color.bodySet.GetBit(bodyIdB)) { continue; }

			color.bodySet.SetBitGrow(bodyIdA);
			color.bodySet.SetBitGrow(bodyIdB);
			colorIndex = i;
			break;
		}
	}
	else if (staticA == false)
	{
		// No static contacts in color 0
		for (int i = 1; i < OverflowIndex; ++i)
		{
			GraphColor& color = Physics::constraintGraph.colors[i];
			if (color.bodySet.GetBit(bodyIdA)) { continue; }

			color.bodySet.SetBitGrow(bodyIdA);
			colorIndex = i;
			break;
		}
	}
	else if (staticB == false)
	{
		// No static contacts in color 0
		for (int i = 1; i < OverflowIndex; ++i)
		{
			GraphColor& color = Physics::constraintGraph.colors[i];
			if (color.bodySet.GetBit(bodyIdB)) { continue; }

			color.bodySet.SetBitGrow(bodyIdB);
			colorIndex = i;
			break;
		}
	}

	GraphColor& color = Physics::constraintGraph.colors[colorIndex];
	contact.colorIndex = colorIndex;
	contact.localIndex = (I32)color.contactSims.Size();

	ContactSim& newContact = color.contactSims.Push(contactSim);

	// todo perhaps skip this if the contact is already awake

	if (staticA)
	{
		newContact.bodySimIndexA = NullIndex;
		newContact.invMassA = 0.0f;
		newContact.invIA = 0.0f;
	}
	else
	{
		SolverSet& awakeSet = Physics::solverSets[SET_TYPE_AWAKE];

		int localIndex = bodyA.localIndex;
		newContact.bodySimIndexA = localIndex;

		BodySim& bodySimA = awakeSet.bodySims[localIndex];
		newContact.invMassA = bodySimA.invMass;
		newContact.invIA = bodySimA.invInertia;
	}

	if (staticB)
	{
		newContact.bodySimIndexB = NullIndex;
		newContact.invMassB = 0.0f;
		newContact.invIB = 0.0f;
	}
	else
	{
		SolverSet& awakeSet = Physics::solverSets[SET_TYPE_AWAKE];

		int localIndex = bodyB.localIndex;
		newContact.bodySimIndexB = localIndex;

		BodySim& bodySimB = awakeSet.bodySims[localIndex];
		newContact.invMassB = bodySimB.invMass;
		newContact.invIB = bodySimB.invInertia;
	}
}

void ConstraintGraph::RemoveContact(I32 bodyIdA, I32 bodyIdB, I32 colorIndex, I32 localIndex)
{
	GraphColor& color = Physics::constraintGraph.colors[colorIndex];

	if (colorIndex != OverflowIndex)
	{
		// might clear a bit for a static body, but this has no effect
		color.bodySet.ClearBit(bodyIdA);
		color.bodySet.ClearBit(bodyIdB);
	}

	int movedIndex = color.contactSims.RemoveSwap(localIndex);
	if (movedIndex != NullIndex)
	{
		// Fix index on swapped contact
		ContactSim& movedContactSim = color.contactSims[localIndex];

		// Fix moved contact
		int movedId = movedContactSim.contactId;
		Contact& movedContact = Physics::contacts[movedId];
		movedContact.localIndex = localIndex;
	}
}

I32 ConstraintGraph::AssignJointColor(I32 bodyIdA, I32 bodyIdB, bool staticA, bool staticB)
{
	if (staticA == false && staticB == false)
	{
		for (int i = 0; i < OverflowIndex; ++i)
		{
			GraphColor& color = Physics::constraintGraph.colors[i];
			if (color.bodySet.GetBit(bodyIdA) || color.bodySet.GetBit(bodyIdB)) { continue; }

			color.bodySet.SetBitGrow(bodyIdA);
			color.bodySet.SetBitGrow(bodyIdB);
			return i;
		}
	}
	else if (staticA == false)
	{
		for (int i = 0; i < OverflowIndex; ++i)
		{
			GraphColor& color = Physics::constraintGraph.colors[i];
			if (color.bodySet.GetBit(bodyIdA)) { continue; }

			color.bodySet.SetBitGrow(bodyIdA);
			return i;
		}
	}
	else if (staticB == false)
	{
		for (int i = 0; i < OverflowIndex; ++i)
		{
			GraphColor& color = Physics::constraintGraph.colors[i];
			if (color.bodySet.GetBit(bodyIdB)) { continue; }

			color.bodySet.SetBitGrow(bodyIdB);
			return i;
		}
	}

	return OverflowIndex;
}

void ConstraintGraph::AddJoint(JointSim& jointSim, Joint& joint)
{
	int bodyIdA = joint.edges[0].bodyId;
	int bodyIdB = joint.edges[1].bodyId;
	RigidBody2D& bodyA = Physics::bodies->Get(bodyIdA);
	RigidBody2D& bodyB = Physics::bodies->Get(bodyIdB);
	bool staticA = bodyA.setIndex == SET_TYPE_STATIC;
	bool staticB = bodyB.setIndex == SET_TYPE_STATIC;

	int colorIndex = AssignJointColor(bodyIdA, bodyIdB, staticA, staticB);

	colors[colorIndex].jointSims.Push(jointSim);
	joint.colorIndex = colorIndex;
	joint.localIndex = (I32)(colors[colorIndex].jointSims.Size() - 1);
}

void ConstraintGraph::RemoveJoint(I32 bodyIdA, I32 bodyIdB, I32 colorIndex, I32 localIndex)
{
	GraphColor& color = Physics::constraintGraph.colors[colorIndex];

	if (colorIndex != OverflowIndex)
	{
		// May clear static bodies, no effect
		color.bodySet.ClearBit(bodyIdA);
		color.bodySet.ClearBit(bodyIdB);
	}

	int movedIndex = color.jointSims.RemoveSwap(localIndex);;
	if (movedIndex != NullIndex)
	{
		// Fix moved joint
		JointSim& movedJointSim = color.jointSims[localIndex];
		int movedId = movedJointSim.jointId;
		Joint& movedJoint = Physics::joints[movedId];
		movedJoint.localIndex = localIndex;
	}
}

//FILTER

bool Filter::ShouldShapesCollide(const Filter& other)
{
	if (groupIndex == other.groupIndex && groupIndex != 0) { return groupIndex > 0; }
	return (layerMask & other.layers) != 0 && (layers & other.layerMask) != 0;
}

//SOFTNESS

void Softness::Create(F32 hertz, F32 zeta, F32 h)
{
	if (hertz == 0.0f)
	{
		biasRate = 0.0f;
		massScale = 1.0f;
		impulseScale = 0.0f;
	}
	else
	{
		F32 omega = 2.0f * PI_F * hertz;
		F32 a1 = 2.0f * zeta + h * omega;
		F32 a2 = h * omega * a1;
		F32 a3 = 1.0f / (1.0f + a2);

		biasRate = omega / a1;
		massScale = a2 * a3;
		impulseScale = a3;
	}
}

//SWEEP

void Sweep::Create(const BodySim& bodySim)
{
	c1 = bodySim.center0;
	c2 = bodySim.center;
	q1 = bodySim.rotation0;
	q2 = bodySim.transform.rotation;
	localCenter = bodySim.localCenter;
}

//SOLVER SET

void SolverSet::Destroy()
{
	bodySims.Destroy();
	bodyStates.Destroy();
	contactSims.Destroy();
	jointSims.Destroy();
	islandSims.Destroy();
	setIndex = NullIndex;
}

//RIGIDBODY2D

RigidBody2D::RigidBody2D(const RigidBody2DDef& def)
{
	//Physics::CreateRigidBody2D();
}

void RigidBody2D::Update(Scene* scene)
{

}

void RigidBody2D::Load(Scene* scene)
{

}

void RigidBody2D::Cleanup(Scene* scene)
{

}

//JOINT SIM

JointSim::JointSim() = default;

JointSim::JointSim(const JointSim& other) : jointId(other.jointId), bodyIdA(other.bodyIdA), bodyIdB(bodyIdB), type(other.type),
	localOriginAnchorA(other.localOriginAnchorA), localOriginAnchorB(other.localOriginAnchorB), 
	invMassA(other.invMassA), invMassB(other.invMassB), invIA(other.invIA), invIB(other.invIB)
{
	switch (type)
	{
	case JOINT_TYPE_DISTANCE: distanceJoint = other.distanceJoint;
	case JOINT_TYPE_MOTOR: motorJoint = other.motorJoint;
	case JOINT_TYPE_MOUSE: mouseJoint = other.mouseJoint;
	case JOINT_TYPE_PRISMATIC: revoluteJoint = other.revoluteJoint;
	case JOINT_TYPE_REVOLUTE: prismaticJoint = other.prismaticJoint;
	case JOINT_TYPE_WELD: weldJoint = other.weldJoint;
	case JOINT_TYPE_WHEEL: wheelJoint = other.wheelJoint;
	}
}

JointSim::JointSim(JointSim&& other) noexcept : jointId(other.jointId), bodyIdA(other.bodyIdA), bodyIdB(bodyIdB), type(other.type),
localOriginAnchorA(other.localOriginAnchorA), localOriginAnchorB(other.localOriginAnchorB),
invMassA(other.invMassA), invMassB(other.invMassB), invIA(other.invIA), invIB(other.invIB)
{
	switch (type)
	{
	case JOINT_TYPE_DISTANCE: distanceJoint = Move(other.distanceJoint);
	case JOINT_TYPE_MOTOR: motorJoint = Move(other.motorJoint);
	case JOINT_TYPE_MOUSE: mouseJoint = Move(other.mouseJoint);
	case JOINT_TYPE_PRISMATIC: revoluteJoint = Move(other.revoluteJoint);
	case JOINT_TYPE_REVOLUTE: prismaticJoint = Move(other.prismaticJoint);
	case JOINT_TYPE_WELD: weldJoint = Move(other.weldJoint);
	case JOINT_TYPE_WHEEL: wheelJoint = Move(other.wheelJoint);
	}
}

JointSim& JointSim::operator=(const JointSim& other)
{
	jointId = other.jointId;
	bodyIdA = other.bodyIdA;
	bodyIdB = bodyIdB;
	type = other.type;
	localOriginAnchorA = other.localOriginAnchorA;
	localOriginAnchorB = other.localOriginAnchorB;
	invMassA = other.invMassA;
	invMassB = other.invMassB;
	invIA = other.invIA;
	invIB = other.invIB;

	switch (type)
	{
	case JOINT_TYPE_DISTANCE: distanceJoint = other.distanceJoint;
	case JOINT_TYPE_MOTOR: motorJoint = other.motorJoint;
	case JOINT_TYPE_MOUSE: mouseJoint = other.mouseJoint;
	case JOINT_TYPE_PRISMATIC: revoluteJoint = other.revoluteJoint;
	case JOINT_TYPE_REVOLUTE: prismaticJoint = other.prismaticJoint;
	case JOINT_TYPE_WELD: weldJoint = other.weldJoint;
	case JOINT_TYPE_WHEEL: wheelJoint = other.wheelJoint;
	}

	return *this;
}

JointSim& JointSim::operator=(JointSim&& other) noexcept
{
	jointId = other.jointId;
	bodyIdA = other.bodyIdA;
	bodyIdB = bodyIdB;
	type = other.type;
	localOriginAnchorA = other.localOriginAnchorA;
	localOriginAnchorB = other.localOriginAnchorB;
	invMassA = other.invMassA;
	invMassB = other.invMassB;
	invIA = other.invIA;
	invIB = other.invIB;

	switch (type)
	{
	case JOINT_TYPE_DISTANCE: distanceJoint = Move(other.distanceJoint);
	case JOINT_TYPE_MOTOR: motorJoint = Move(other.motorJoint);
	case JOINT_TYPE_MOUSE: mouseJoint = Move(other.mouseJoint);
	case JOINT_TYPE_PRISMATIC: revoluteJoint = Move(other.revoluteJoint);
	case JOINT_TYPE_REVOLUTE: prismaticJoint = Move(other.prismaticJoint);
	case JOINT_TYPE_WELD: weldJoint = Move(other.weldJoint);
	case JOINT_TYPE_WHEEL: wheelJoint = Move(other.wheelJoint);
	}

	return *this;
}

//SHAPE

Shape::Shape(const ShapeDef& def)
{

}

Shape::Shape(const Shape& other)
{

}

Shape::Shape(Shape&& other) noexcept
{

}

Shape& Shape::operator=(const Shape& other)
{

}

Shape& Shape::operator=(Shape&& other) noexcept
{

}