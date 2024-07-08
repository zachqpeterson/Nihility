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

#include "Island.hpp"

#include "Physics.hpp"

WorldManifold::WorldManifold(const Manifold2D* manifold,
	const Transform2D& xfA, F32 radiusA,
	const Transform2D& xfB, F32 radiusB)
{
	if (manifold->pointCount == 0) { return; }

	switch (manifold->type)
	{
	case Manifold2D::Circles: {
		normal = { 1.0f, 0.0f };
		Vector2 pointA = manifold->localPoint * xfA;
		Vector2 pointB = manifold->points[0].localPoint * xfB;
		if ((pointA - pointB).SqrMagnitude() > Traits<F32>::Epsilon * Traits<F32>::Epsilon)
		{
			normal = pointB - pointA;
			normal.Normalize();
		}

		Vector2 cA = pointA + radiusA * normal;
		Vector2 cB = pointB - radiusB * normal;
		points[0] = 0.5f * (cA + cB);
		separations[0] = (cB - cA).Dot(normal);
	} break;
	case Manifold2D::FaceA: {
		normal = manifold->localNormal * xfA.rotation;
		Vector2 planePoint = manifold->localPoint * xfA;

		for (U32 i = 0; i < manifold->pointCount; ++i)
		{
			Vector2 clipPoint = manifold->points[i].localPoint * xfB;
			Vector2 cA = clipPoint + (radiusA - (clipPoint - planePoint).Dot(normal)) * normal;
			Vector2 cB = clipPoint - radiusB * normal;
			points[i] = 0.5f * (cA + cB);
			separations[i] = (cB - cA).Dot(normal);
		}
	} break;
	case Manifold2D::FaceB: {
		normal = manifold->localNormal * xfB.rotation;
		Vector2 planePoint = manifold->localPoint * xfB;

		for (U32 i = 0; i < manifold->pointCount; ++i)
		{
			Vector2 clipPoint = manifold->points[i].localPoint * xfA;
			Vector2 cB = clipPoint + (radiusB - (clipPoint - planePoint).Dot(normal)) * normal;
			Vector2 cA = clipPoint - radiusA * normal;
			points[i] = 0.5f * (cA + cB);
			separations[i] = (cA - cB).Dot(normal);
		}

		// Ensure normal points from A to B.
		normal = -normal;
	} break;
	}
}

void Island2D::Solve(F32 step)
{
	for (U64 i = 0; i < bodies.Size(); ++i)
	{
		RigidBody2D* rb = bodies[i];

		Vector2 c = rb->sweep.c;
		F32 a = rb->sweep.a;
		Vector2 v = rb->velocity;
		F32 w = rb->angularVelocity;

		rb->sweep.c0 = rb->sweep.c;
		rb->sweep.a0 = rb->sweep.a;

		if (rb->type == BODY_TYPE_DYNAMIC)
		{
			v += (rb->gravityScale * Physics::Gravity().xy() + rb->force * rb->invMass) * step;
			w += rb->torque * rb->invInertia * step;

			v *= 1.0f / (1.0f + rb->linearDrag * step);
			w *= 1.0f / (1.0f + rb->angularDrag * step);
		}

		positions[i].c = c;
		positions[i].a = a;
		velocities[i].v = v;
		velocities[i].w = w;
	}

	SetupConstraints();
	InitVelocityConstraints();
	WarmStart();

	//Solve Velocity Constraints
	for (U32 i = 0; i < VelocityIterations; ++i)
	{
		SolveVelocityConstraints();
	}

	for (U32 i = 0; i < contacts.Size(); ++i)
	{
		ContactVelocityConstraint& cvc = velocityConstraints[i];
		Manifold2D& manifold = contacts[cvc.contactIndex]->manifold;

		for (U32 j = 0; j < cvc.pointCount; ++j)
		{
			manifold.points[j].normalImpulse = cvc.points[j].normalImpulse;
			manifold.points[j].tangentImpulse = cvc.points[j].tangentImpulse;
		}
	}

	// Integrate positions
	for (U32 i = 0; i < bodies.Size(); ++i)
	{
		Vector2 c = positions[i].c;
		F32 a = positions[i].a;
		Vector2 v = velocities[i].v;
		F32 w = velocities[i].w;

		// Check for large velocities
		Vector2 translation = v * step;
		if (translation.SqrMagnitude() > MaxTranslationSquared)
		{
			F32 ratio = MaxTranslation / (translation.Magnitude());
			v *= ratio;
		}

		F32 rotation = w * step;
		if (rotation * rotation > MaxRotationSquared)
		{
			F32 ratio = MaxRotation / Math::Abs(rotation);
			w *= ratio;
		}

		// Integrate
		c += v * step;
		a += w * step;

		positions[i].c = c;
		positions[i].a = a;
		velocities[i].v = v;
		velocities[i].w = w;
	}

	// Solve position constraints
	bool positionSolved = false;
	for (U32 i = 0; i < PositionIterations; ++i)
	{
		if (SolvePositionConstraints())
		{
			// Exit early if the position errors are small.
			positionSolved = true;
			break;
		}
	}

	// Copy state buffers back to the bodies
	U32 i = 0;
	for (RigidBody2D* rb : bodies)
	{
		rb->sweep.c = positions[i].c;
		rb->sweep.a = positions[i].a;
		rb->velocity = velocities[i].v;
		rb->angularVelocity = velocities[i].w;
		rb->SynchronizeTransform();

		++i;
	}

	//TODO: Events

	F32 minSleepTime = F32_MAX;

	for (RigidBody2D* rb : bodies)
	{
		if (rb->type == BODY_TYPE_STATIC) { continue; }

		if ((rb->flags & RigidBody2D::FLAG_AUTO_SLEEP) == 0 ||
			rb->angularVelocity * rb->angularVelocity > AngularSleepToleranceSqr ||
			rb->velocity.SqrMagnitude() > LinearSleepToleranceSqr)
		{
			rb->sleepTime = 0.0f;
			minSleepTime = 0.0f;
		}
		else
		{
			rb->sleepTime += step;
			minSleepTime = Math::Min(minSleepTime, rb->sleepTime);
		}
	}

	if (minSleepTime >= TimeToSleep && positionSolved)
	{
		for (RigidBody2D* rb : bodies) { rb->SetAwake(false); }
	}
}

void Island2D::SolveTOI(F32 step, U32 toiIndexA, U32 toiIndexB)
{
	// Initialize the body state.
	U32 i = 0;
	for (RigidBody2D* rb : bodies)
	{
		positions[i].c = rb->sweep.c;
		positions[i].a = rb->sweep.a;
		velocities[i].v = rb->velocity;
		velocities[i].w = rb->angularVelocity;

		++i;
	}

	SetupConstraints();

	// Solve position constraints.
	for (U32 i = 0; i < PositionIterations; ++i)
	{
		if (SolveTOIPositionConstraints(toiIndexA, toiIndexB)) { break; }
	}

	// Leap of faith to new safe state.
	bodies[toiIndexA]->sweep.c0 = positions[toiIndexA].c;
	bodies[toiIndexA]->sweep.a0 = positions[toiIndexA].a;
	bodies[toiIndexB]->sweep.c0 = positions[toiIndexB].c;
	bodies[toiIndexB]->sweep.a0 = positions[toiIndexB].a;

	InitVelocityConstraints();

	// Solve velocity constraints.
	for (U32 i = 0; i < VelocityIterations; ++i)
	{
		SolveVelocityConstraints();
	}

	// Integrate positions
	for (U32 i = 0; i < bodies.Size(); ++i)
	{
		Vector2 c = positions[i].c;
		F32 a = positions[i].a;
		Vector2 v = velocities[i].v;
		F32 w = velocities[i].w;

		// Check for large velocities
		Vector2 translation = v * step;
		if (translation.SqrMagnitude() > MaxTranslationSquared)
		{
			F32 ratio = MaxTranslation / translation.Magnitude();
			v *= ratio;
		}

		F32 rotation = w * step;
		if (rotation * rotation > MaxRotationSquared)
		{
			F32 ratio = MaxRotation / Math::Abs(rotation);
			w *= ratio;
		}

		// Integrate
		c += v * step;
		a += w * step;

		positions[i].c = c;
		positions[i].a = a;
		velocities[i].v = v;
		velocities[i].w = w;

		// Sync bodies
		RigidBody2D* body = bodies[i];
		body->sweep.c = c;
		body->sweep.a = a;
		body->velocity = v;
		body->angularVelocity = w;
		body->SynchronizeTransform();
	}
	
	//TODO: Events
}

void Island2D::SetupConstraints()
{
	for (Contact2D* contact : contacts)
	{
		Collider2D* colliderA = contact->colliderA.Data();
		Collider2D* colliderB = contact->colliderB.Data();
		F32 radiusA = colliderA->radius;
		F32 radiusB = colliderB->radius;
		RigidBody2D* rbA = colliderA->body.Data();
		RigidBody2D* rbB = colliderB->body.Data();
		Manifold2D* manifold = &contact->manifold;

		U32 pointCount = manifold->pointCount;

		U32 index = (U32)velocityConstraints.Size();
		ContactVelocityConstraint& cvc = velocityConstraints.Push({});
		cvc.friction = contact->friction;
		cvc.restitution = contact->restitution;
		cvc.tangentSpeed = contact->tangentSpeed;
		cvc.indexA = rbA->islandIndex;
		cvc.indexB = rbB->islandIndex;
		cvc.invMassA = rbA->invMass;
		cvc.invMassB = rbB->invMass;
		cvc.invIA = rbA->invInertia;
		cvc.invIB = rbB->invInertia;
		cvc.contactIndex = index;
		cvc.pointCount = pointCount;

		ContactPositionConstraint& cpc = positionConstraints.Push({});
		cpc.indexA = rbA->islandIndex;
		cpc.indexB = rbB->islandIndex;
		cpc.invMassA = rbA->invMass;
		cpc.invMassB = rbB->invMass;
		cpc.localCenterA = rbA->sweep.localCenter;
		cpc.localCenterB = rbB->sweep.localCenter;
		cpc.invIA = rbA->invInertia;
		cpc.invIB = rbB->invInertia;
		cpc.localNormal = manifold->localNormal;
		cpc.localPoint = manifold->localPoint;
		cpc.pointCount = pointCount;
		cpc.radiusA = radiusA;
		cpc.radiusB = radiusB;
		cpc.type = manifold->type;

		for (U32 i = 0; i < pointCount; ++i)
		{
			ManifoldPoint& mp = manifold->points[i];
			VelocityConstraintPoint& vcp = cvc.points[i];

			vcp.normalImpulse = mp.normalImpulse;
			vcp.tangentImpulse = mp.tangentImpulse;
			vcp.rA = Vector2Zero;
			vcp.rB = Vector2Zero;
			vcp.normalMass = 0.0f;
			vcp.tangentMass = 0.0f;
			vcp.velocityBias = 0.0f;

			cpc.localPoints[i] = mp.localPoint;
		}
	}
}

void Island2D::InitVelocityConstraints()
{
	for (U32 i = 0; i < contacts.Size(); ++i)
	{
		ContactVelocityConstraint& cvc = velocityConstraints[i];
		ContactPositionConstraint& cpc = positionConstraints[i];

		F32 radiusA = cpc.radiusA;
		F32 radiusB = cpc.radiusB;
		Manifold2D* manifold = &contacts[cvc.contactIndex]->manifold;

		U32 indexA = cvc.indexA;
		U32 indexB = cvc.indexB;

		F32 mA = cvc.invMassA;
		F32 mB = cvc.invMassB;
		F32 iA = cvc.invIA;
		F32 iB = cvc.invIB;
		Vector2 localCenterA = cpc.localCenterA;
		Vector2 localCenterB = cpc.localCenterB;

		Vector2 cA = positions[indexA].c;
		F32 aA = positions[indexA].a;
		Vector2 vA = velocities[indexA].v;
		F32 wA = velocities[indexA].w;

		Vector2 cB = positions[indexB].c;
		F32 aB = positions[indexB].a;
		Vector2 vB = velocities[indexB].v;
		F32 wB = velocities[indexB].w;

		Transform2D xfA, xfB;
		xfA.rotation = aA;
		xfB.rotation = aB;
		xfA.position = cA - localCenterA * xfA.rotation;
		xfB.position = cB - localCenterB * xfB.rotation;

		WorldManifold worldManifold(manifold, xfA, radiusA, xfB, radiusB);

		cvc.normal = worldManifold.normal;

		U32 pointCount = cvc.pointCount;
		for (U32 i = 0; i < pointCount; ++i)
		{
			VelocityConstraintPoint& vcp = cvc.points[i];

			vcp.rA = worldManifold.points[i] - cA;
			vcp.rB = worldManifold.points[i] - cB;

			F32 rnA = vcp.rA.Cross(cvc.normal);
			F32 rnB = vcp.rB.Cross(cvc.normal);

			F32 kNormal = mA + mB + iA * rnA * rnA + iB * rnB * rnB;

			vcp.normalMass = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

			Vector2 tangent = cvc.normal.Cross(1.0f);

			F32 rtA = vcp.rA.Cross(tangent);
			F32 rtB = vcp.rB.Cross(tangent);

			F32 kTangent = mA + mB + iA * rtA * rtA + iB * rtB * rtB;

			vcp.tangentMass = kTangent > 0.0f ? 1.0f / kTangent : 0.0f;

			// Setup a velocity bias for restitution.
			vcp.velocityBias = 0.0f;
			F32 vRel = cvc.normal.Dot(vB + vcp.rB.CrossInv(wB) - vA - vcp.rA.CrossInv(wA));
			if (vRel < -VelocityThreshold)
			{
				vcp.velocityBias = -cvc.restitution * vRel;
			}
		}

		// If we have two points, then prepare the block solver.
		if (cvc.pointCount == 2)
		{
			VelocityConstraintPoint& vcp1 = cvc.points[0];
			VelocityConstraintPoint& vcp2 = cvc.points[1];

			F32 rn1A = vcp1.rA.Cross(cvc.normal);
			F32 rn1B = vcp1.rB.Cross(cvc.normal);
			F32 rn2A = vcp2.rA.Cross(cvc.normal);
			F32 rn2B = vcp2.rB.Cross(cvc.normal);

			F32 k11 = mA + mB + iA * rn1A * rn1A + iB * rn1B * rn1B;
			F32 k22 = mA + mB + iA * rn2A * rn2A + iB * rn2B * rn2B;
			F32 k12 = mA + mB + iA * rn1A * rn2A + iB * rn1B * rn2B;

			// Ensure a reasonable condition number.
			const F32 k_maxConditionNumber = 1000.0f;
			if (k11 * k11 < k_maxConditionNumber * (k11 * k22 - k12 * k12))
			{
				// K is safe to invert.
				cvc.K.a = { k11, k12 };
				cvc.K.b = { k12, k22 };
				cvc.normalMass = cvc.K.Inversed();
			}
			else
			{
				// The constraints are redundant, just use one.
				// TODO_ERIN use deepest?
				cvc.pointCount = 1;
			}
		}
	}
}

void Island2D::WarmStart()
{
	for (U32 i = 0; i < contacts.Size(); ++i)
	{
		ContactVelocityConstraint& cvc = velocityConstraints[i];
		ContactPositionConstraint& cpc = positionConstraints[i];

		U32 indexA = cvc.indexA;
		U32 indexB = cvc.indexB;
		F32 mA = cvc.invMassA;
		F32 iA = cvc.invIA;
		F32 mB = cvc.invMassB;
		F32 iB = cvc.invIB;
		U32 pointCount = cvc.pointCount;

		Vector2 vA = velocities[indexA].v;
		F32 wA = velocities[indexA].w;
		Vector2 vB = velocities[indexB].v;
		F32 wB = velocities[indexB].w;

		Vector2 normal = cvc.normal;
		Vector2 tangent = normal.Cross(1.0f);

		for (U32 i = 0; i < pointCount; ++i)
		{
			VelocityConstraintPoint& vcp = cvc.points[i];
			Vector2 P = vcp.normalImpulse * normal + vcp.tangentImpulse * tangent;
			wA -= iA * vcp.rA.Cross(P);
			vA -= mA * P;
			wB += iB * vcp.rB.Cross(P);
			vB += mB * P;
		}

		velocities[indexA].v = vA;
		velocities[indexA].w = wA;
		velocities[indexB].v = vB;
		velocities[indexB].w = wB;
	}
}

void Island2D::SolveVelocityConstraints()
{
	for (U32 i = 0; i < contacts.Size(); ++i)
	{
		ContactVelocityConstraint& cvc = velocityConstraints[i];

		U32 indexA = cvc.indexA;
		U32 indexB = cvc.indexB;
		F32 mA = cvc.invMassA;
		F32 iA = cvc.invIA;
		F32 mB = cvc.invMassB;
		F32 iB = cvc.invIB;
		U32 pointCount = cvc.pointCount;

		Vector2 vA = velocities[indexA].v;
		F32 wA = velocities[indexA].w;
		Vector2 vB = velocities[indexB].v;
		F32 wB = velocities[indexB].w;

		Vector2 normal = cvc.normal;
		Vector2 tangent = normal.Cross(1.0f);
		F32 friction = cvc.friction;

		// Solve tangent constraints first because non-penetration is more important than friction.
		for (U32 j = 0; j < pointCount; ++j)
		{
			VelocityConstraintPoint& vcp = cvc.points[j];

			// Relative velocity at contact
			Vector2 dv = vB + vcp.rB.CrossInv(wB) - vA - vcp.rA.CrossInv(wA);

			// Compute tangent force
			F32 vt = dv.Dot(tangent) - cvc.tangentSpeed;
			F32 lambda = vcp.tangentMass * (-vt);

			// b2::Clamp the accumulated force
			F32 maxFriction = friction * vcp.normalImpulse;
			F32 newImpulse = Math::Clamp(vcp.tangentImpulse + lambda, -maxFriction, maxFriction);
			lambda = newImpulse - vcp.tangentImpulse;
			vcp.tangentImpulse = newImpulse;

			// Apply contact impulse
			Vector2 P = lambda * tangent;

			vA -= mA * P;
			wA -= iA * vcp.rA.Cross(P);

			vB += mB * P;
			wB += iB * vcp.rB.Cross(P);
		}

		// Solve normal constraints
		if (pointCount == 1)
		{
			for (U32 i = 0; i < pointCount; ++i)
			{
				VelocityConstraintPoint& vcp = cvc.points[i];

				// Relative velocity at contact
				Vector2 dv = vB + vcp.rB.CrossInv(wB) - vA - vcp.rA.CrossInv(wA);

				// Compute normal impulse
				F32 vn = dv.Dot(normal);
				F32 lambda = -vcp.normalMass * (vn - vcp.velocityBias);

				// b2::Clamp the accumulated impulse
				F32 newImpulse = Math::Max(vcp.normalImpulse + lambda, 0.0f);
				lambda = newImpulse - vcp.normalImpulse;
				vcp.normalImpulse = newImpulse;

				// Apply contact impulse
				Vector2 P = lambda * normal;
				vA -= mA * P;
				wA -= iA * vcp.rA.Cross(P);

				vB += mB * P;
				wB += iB * vcp.rB.Cross(P);
			}
		}
		else
		{
			VelocityConstraintPoint* cp1 = cvc.points + 0;
			VelocityConstraintPoint* cp2 = cvc.points + 1;

			Vector2 a(cp1->normalImpulse, cp2->normalImpulse);

			// Relative velocity at contact
			Vector2 dv1 = vB + cp1->rB.CrossInv(wB) - vA - cp1->rA.CrossInv(wA);
			Vector2 dv2 = vB + cp2->rB.CrossInv(wB) - vA - cp2->rA.CrossInv(wA);

			// Compute normal velocity
			F32 vn1 = dv1.Dot(normal);
			F32 vn2 = dv2.Dot(normal);

			Vector2 b;
			b.x = vn1 - cp1->velocityBias;
			b.y = vn2 - cp2->velocityBias;

			// Compute b'
			b -= cvc.K * a;

			for (;;)
			{
				Vector2 x = -(cvc.normalMass * b);

				if (x.x >= 0.0f && x.y >= 0.0f)
				{
					// Get the incremental impulse
					Vector2 d = x - a;

					// Apply incremental impulse
					Vector2 P1 = d.x * normal;
					Vector2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
					wA -= iA * (cp1->rA.Cross(P1) + cp2->rA.Cross(P2));

					vB += mB * (P1 + P2);
					wB += iB * (cp1->rB.Cross(P1) + cp2->rB.Cross(P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

					break;
				}

				x.x = -cp1->normalMass * b.x;
				x.y = 0.0f;
				vn1 = 0.0f;
				vn2 = cvc.K.a.y * x.x + b.y;

				if (x.x >= 0.0f && vn2 >= 0.0f)
				{
					// Get the incremental impulse
					Vector2 d = x - a;

					// Apply incremental impulse
					Vector2 P1 = d.x * normal;
					Vector2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
					wA -= iA * (cp1->rA.Cross(P1) + cp2->rA.Cross(P2));

					vB += mB * (P1 + P2);
					wB += iB * (cp1->rB.Cross(P1) + cp2->rB.Cross(P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

					break;
				}

				x.x = 0.0f;
				x.y = -cp2->normalMass * b.y;
				vn1 = cvc.K.b.x * x.y + b.x;
				vn2 = 0.0f;

				if (x.y >= 0.0f && vn1 >= 0.0f)
				{
					// Resubstitute for the incremental impulse
					Vector2 d = x - a;

					// Apply incremental impulse
					Vector2 P1 = d.x * normal;
					Vector2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
					wA -= iA * (cp1->rA.Cross(P1) + cp2->rA.Cross(P2));

					vB += mB * (P1 + P2);
					wB += iB * (cp1->rB.Cross(P1) + cp2->rB.Cross(P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

					break;
				}

				x.x = 0.0f;
				x.y = 0.0f;
				vn1 = b.x;
				vn2 = b.y;

				if (vn1 >= 0.0f && vn2 >= 0.0f)
				{
					// Resubstitute for the incremental impulse
					Vector2 d = x - a;

					// Apply incremental impulse
					Vector2 P1 = d.x * normal;
					Vector2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
					wA -= iA * (cp1->rA.Cross(P1) + cp2->rA.Cross(P2));

					vB += mB * (P1 + P2);
					wB += iB * (cp1->rB.Cross(P1) + cp2->rB.Cross(P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

					break;
				}

				// No solution, give up. This is hit sometimes, but it doesn't seem to matter.
				break;
			}
		}

		velocities[indexA].v = vA;
		velocities[indexA].w = wA;
		velocities[indexB].v = vB;
		velocities[indexB].w = wB;
	}
}

bool Island2D::SolvePositionConstraints()
{
	F32 minSeparation = 0.0f;

	for (U32 i = 0; i < contacts.Size(); ++i)
	{
		ContactPositionConstraint& cpc = positionConstraints[i];

		U32 indexA = cpc.indexA;
		U32 indexB = cpc.indexB;
		Vector2 localCenterA = cpc.localCenterA;
		F32 mA = cpc.invMassA;
		F32 iA = cpc.invIA;
		Vector2 localCenterB = cpc.localCenterB;
		F32 mB = cpc.invMassB;
		F32 iB = cpc.invIB;
		U32 pointCount = cpc.pointCount;

		Vector2 cA = positions[indexA].c;
		F32 aA = positions[indexA].a;

		Vector2 cB = positions[indexB].c;
		F32 aB = positions[indexB].a;

		// Solve normal constraints
		for (U32 j = 0; j < pointCount; ++j)
		{
			Transform2D xfA, xfB;
			xfA.rotation = aA;
			xfB.rotation = aB;
			xfA.position = cA - localCenterA * xfA.rotation;
			xfB.position = cB - localCenterB * xfB.rotation;

			PositionSolverManifold psm(cpc, xfA, xfB, j);
			Vector2 normal = psm.normal;

			Vector2 point = psm.point;
			F32 separation = psm.separation;

			Vector2 rA = point - cA;
			Vector2 rB = point - cB;

			// Track max constraint error.
			minSeparation = Math::Min(minSeparation, separation);

			// Prevent large corrections and allow slop.
			F32 C = Math::Clamp(Baumgarte * (separation + LinearSlop), -MaxLinearCorrection, 0.0f);

			// Compute the effective mass.
			F32 rnA = rA.Cross(normal);
			F32 rnB = rB.Cross(normal);
			F32 K = mA + mB + iA * rnA * rnA + iB * rnB * rnB;

			// Compute normal impulse
			F32 impulse = K > 0.0f ? -C / K : 0.0f;

			Vector2 P = impulse * normal;

			cA -= mA * P;
			aA -= iA * rA.Cross(P);

			cB += mB * P;
			aB += iB * rB.Cross(P);
		}

		positions[indexA].c = cA;
		positions[indexA].a = aA;

		positions[indexB].c = cB;
		positions[indexB].a = aB;
	}

	return minSeparation >= -3.0f * LinearSlop;
}

bool Island2D::SolveTOIPositionConstraints(U32 toiIndexA, U32 toiIndexB)
{
	F32 minSeparation = 0.0f;

	for (U32 i = 0; i < contacts.Size(); ++i)
	{
		ContactPositionConstraint& cpc = positionConstraints[i];

		U32 indexA = cpc.indexA;
		U32 indexB = cpc.indexB;
		Vector2 localCenterA = cpc.localCenterA;
		Vector2 localCenterB = cpc.localCenterB;
		U32 pointCount = cpc.pointCount;

		F32 mA = 0.0f;
		F32 iA = 0.0f;
		if (indexA == toiIndexA || indexA == toiIndexB)
		{
			mA = cpc.invMassA;
			iA = cpc.invIA;
		}

		F32 mB = 0.0f;
		F32 iB = 0.;
		if (indexB == toiIndexA || indexB == toiIndexB)
		{
			mB = cpc.invMassB;
			iB = cpc.invIB;
		}

		Vector2 cA = positions[indexA].c;
		F32 aA = positions[indexA].a;

		Vector2 cB = positions[indexB].c;
		F32 aB = positions[indexB].a;

		// Solve normal constraints
		for (U32 j = 0; j < pointCount; ++j)
		{
			Transform2D xfA, xfB;
			xfA.rotation = aA;
			xfB.rotation = aB;
			xfA.position = cA - localCenterA * xfA.rotation;
			xfB.position = cB - localCenterB * xfB.rotation;

			PositionSolverManifold psm(cpc, xfA, xfB, j);
			Vector2 normal = psm.normal;

			Vector2 point = psm.point;
			F32 separation = psm.separation;

			Vector2 rA = point - cA;
			Vector2 rB = point - cB;

			// Track max constraint error.
			minSeparation = Math::Min(minSeparation, separation);

			// Prevent large corrections and allow slop.
			F32 C = Math::Clamp(TOIBaugarte * (separation + LinearSlop), -MaxLinearCorrection, 0.0f);

			// Compute the effective mass.
			F32 rnA = rA.Cross(normal);
			F32 rnB = rB.Cross(normal);
			F32 K = mA + mB + iA * rnA * rnA + iB * rnB * rnB;

			// Compute normal impulse
			F32 impulse = K > 0.0f ? -C / K : 0.0f;

			Vector2 P = impulse * normal;

			cA -= mA * P;
			aA -= iA * rA.Cross(P);

			cB += mB * P;
			aB += iB * rB.Cross(P);
		}

		positions[indexA].c = cA;
		positions[indexA].a = aA;

		positions[indexB].c = cB;
		positions[indexB].a = aB;
	}

	return minSeparation >= -1.5f * LinearSlop;
}