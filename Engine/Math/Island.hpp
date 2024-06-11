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

struct Position
{
	Vector2 c;
	F32 a;
};

struct Velocity
{
	Vector2 v;
	F32 w;
};

struct ContactPositionConstraint
{
	Vector2 localPoints[MaxManifoldPoints];
	Vector2 localNormal;
	Vector2 localPoint;
	U32 indexA;
	U32 indexB;
	F32 invMassA, invMassB;
	Vector2 localCenterA, localCenterB;
	F32 invIA, invIB;
	Manifold2D::Type type;
	F32 radiusA, radiusB;
	U32 pointCount;
};

struct VelocityConstraintPoint
{
	Vector2 rA;
	Vector2 rB;
	F32 normalImpulse;
	F32 tangentImpulse;
	F32 normalMass;
	F32 tangentMass;
	F32 velocityBias;
};

struct ContactVelocityConstraint
{
	VelocityConstraintPoint points[MaxManifoldPoints];
	Vector2 normal;
	Matrix2 normalMass{ Matrix2Zero };
	Matrix2 K{ Matrix2Zero };
	U32 indexA;
	U32 indexB;
	F32 invMassA, invMassB;
	F32 invIA, invIB;
	F32 friction;
	F32 restitution;
	F32 tangentSpeed;
	U32 pointCount;
	U32 contactIndex;
};

struct WorldManifold
{
	WorldManifold(const Manifold2D* manifold,
		const Transform2D& xfA, F32 radiusA,
		const Transform2D& xfB, F32 radiusB);

	Vector2 normal;
	Vector2 points[MaxManifoldPoints];
	F32 separations[MaxManifoldPoints];
};

struct PositionSolverManifold
{
	PositionSolverManifold(ContactPositionConstraint& cpc, const Transform2D& xfA, const Transform2D& xfB, U32 index)
	{
		switch (cpc.type)
		{
		case Manifold2D::Circles: {
			Vector2 pointA = cpc.localPoint * xfA;
			Vector2 pointB = cpc.localPoints[0] * xfB;
			normal = pointB - pointA;
			normal.Normalize();
			point = 0.5f * (pointA + pointB);
			separation = (pointB - pointA).Dot(normal) - cpc.radiusA - cpc.radiusB;
		} break;

		case Manifold2D::FaceA: {
			normal = cpc.localNormal * xfA.rotation;
			Vector2 planePoint = cpc.localPoint * xfA;

			Vector2 clipPoint = cpc.localPoints[index] * xfB;
			separation = (clipPoint - planePoint).Dot(normal) - cpc.radiusA - cpc.radiusB;
			point = clipPoint;
		} break;

		case Manifold2D::FaceB: {
			normal = cpc.localNormal * xfB.rotation;
			Vector2 planePoint = cpc.localPoint * xfB;

			Vector2 clipPoint = cpc.localPoints[index] * xfA;
			separation = (clipPoint - planePoint).Dot(normal) - cpc.radiusA - cpc.radiusB;
			point = clipPoint;

			// Ensure normal points from A to B
			normal = -normal;
		} break;
		}
	}

	Vector2 normal;
	Vector2 point;
	F32 separation;
};

struct Island2D
{
	Island2D(U64 bodyCount, U64 contactCount) : bodies{ bodyCount }, contacts{ contactCount }, positions{ bodyCount, {} }, velocities{ bodyCount, {} } {}
	~Island2D() { bodies.Cleanup(); contacts.Cleanup(); positions.Cleanup(); velocities.Cleanup(); }

	void Clear() { bodies.Clear(); positions.Clear(); velocities.Clear(); }
	void AddBody(RigidBody2D* body) { body->islandIndex = (U32)bodies.Size(); bodies.Push(body); }
	void AddContact(Contact2D* contact) { contacts.Push(contact); }

	void Solve(F32 step);
	void SolveTOI(F32 step, U32 toiIndexA, U32 toiIndexB);

private:
	void SetupConstraints();
	void InitVelocityConstraints();
	void WarmStart();
	void SolveVelocityConstraints();
	bool SolvePositionConstraints();
	bool SolveTOIPositionConstraints(U32 toiIndexA, U32 toiIndexB);

	Vector<RigidBody2D*> bodies;
	Vector<Contact2D*> contacts;
	Vector<Position> positions;
	Vector<Velocity> velocities;
	Vector<ContactPositionConstraint> positionConstraints;
	Vector<ContactVelocityConstraint> velocityConstraints;

	friend class Physics;
};