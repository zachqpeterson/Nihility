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
#include "Resources\Scene.hpp"

import Core;
import Containers;

Vector<RigidBody2D>* Physics::bodies;
Contact2D* Physics::contacts;
U64 Physics::contactCapacity;
Freelist Physics::contactFreelist{};
Vector3 Physics::gravity = Vector3Down * 9.8f;
U32 Physics::flags = 0;
bool Physics::stepComplete = true;

bool Physics::Initialize()
{
	Logger::Trace("Initializing Physics...");

	Memory::AllocateArray(&contacts, 128, contactCapacity);
	contactFreelist((U32)contactCapacity);

	Broadphase::Initialize();

	return true;
}

void Physics::Shutdown()
{
	Logger::Trace("Shutting Down Physics...");

	Broadphase::Shutdown();

	Memory::Free(&contacts);
	contactFreelist.Destroy();
}

void Physics::SetScene(Scene* scene)
{
	//TODO: Setup
	scene->RegisterComponent<RigidBody2D>();
	bodies = scene->GetComponentPool<RigidBody2D>();
}

Collider2D Physics::CreateCircleCollider2D(const ColliderInfo& info, F32 radius)
{
	Collider2D collider{};
	collider.type = COLLIDER_TYPE_CIRCLE;
	collider.radius = radius;
	collider.center = info.center;
	collider.trigger = info.trigger;
	collider.layers = info.layers;
	collider.restitution = info.restitution;
	collider.staticFriction = info.staticFriction;
	collider.dynamicFriction = info.dynamicFriction;

	return Move(collider);
}

Collider2D Physics::CreateBoxCollider2D(const ColliderInfo& info, F32 halfWidth, F32 halfHeight)
{
	Collider2D collider{};
	collider.type = COLLIDER_TYPE_POLYGON;
	collider.polygon.vertexCount = 4;
	collider.polygon.vertices[0] = { -halfWidth, -halfHeight };
	collider.polygon.vertices[1] = { halfWidth, -halfHeight };
	collider.polygon.vertices[2] = { halfWidth, halfHeight };
	collider.polygon.vertices[3] = { -halfWidth, halfHeight };
	collider.polygon.normals[0] = { 0.0f, -1.0f };
	collider.polygon.normals[1] = { 1.0f, 0.0f };
	collider.polygon.normals[2] = { 0.0f, 1.0f };
	collider.polygon.normals[3] = { -1.0f, 0.0f };
	collider.radius = Math::Sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
	collider.center = info.center;
	collider.trigger = info.trigger;
	collider.layers = info.layers;
	collider.restitution = info.restitution;
	collider.staticFriction = info.staticFriction;
	collider.dynamicFriction = info.dynamicFriction;

	return Move(collider);
}

Collider2D Physics::CreatePolygonCollider2D(const ColliderInfo& info, U8 vertexCount, Vector2* vertices)
{
	Collider2D collider{};
	collider.type = COLLIDER_TYPE_POLYGON;
	collider.polygon.vertexCount = vertexCount;
	Copy(collider.polygon.vertices, vertices, vertexCount);
	collider.center = info.center;
	collider.trigger = info.trigger;
	collider.layers = info.layers;
	collider.restitution = info.restitution;
	collider.staticFriction = info.staticFriction;
	collider.dynamicFriction = info.dynamicFriction;

	U32 prev = vertexCount - 1;
	for (U32 i = 0; i < vertexCount; ++i)
	{
		Vector2 edge = vertices[i] - vertices[prev];
		collider.polygon.normals[i] = edge.Cross(1.0f).Normalized();
		prev = i;
	}

	return Move(collider);
}

const Vector3& Physics::Gravity()
{
	return gravity;
}

void Physics::Update(F32 step)
{
	if (!bodies || bodies->Empty()) { return; }

	if (flags & FLAG_NEW_FIXTURE)
	{
		Broadphase::UpdatePairs(); //ContactManager
		flags &= ~FLAG_NEW_FIXTURE;
	}

	flags |= FLAG_LOCKED;

	DetectCollision();
	Solve(step);
	SolveTOI(step);

	if (flags & FLAG_CLEAR_FORCES)
	{
		for (RigidBody2D& body : *bodies)
		{
			body.force = Vector2Zero;
			body.torque = 0.0f;
		}
	}

	flags &= ~FLAG_LOCKED;
}

void Physics::Solve(F32 step)
{
	Island2D island(bodies->Size(), contactFreelist.Last());

	for (RigidBody2D& rb : *bodies) { rb.flags &= ~RigidBody2D::FLAG_ISLAND; }
	for (U32 i = 0; i < contactFreelist.Last(); ++i) { contacts[i].flags &= ~Contact2D::FLAG_ISLAND; }

	Stack<RigidBody2D*> stack((U32)bodies->Size());

	for (RigidBody2D& seed : *bodies)
	{
		if (seed.flags & RigidBody2D::FLAG_ISLAND || (seed.flags & RigidBody2D::MASK_AWAKE_ACTIVE) != RigidBody2D::MASK_AWAKE_ACTIVE || seed.type == BODY_TYPE_STATIC) { continue; }

		island.Clear();
		stack.Clear();
		stack.Push(&seed);
		seed.flags |= RigidBody2D::FLAG_ISLAND;

		//Depth first search
		RigidBody2D* rb;
		while (stack.Pop(rb))
		{
			island.AddBody(rb);

			rb->SetAwake(true);

			if (rb->type == BODY_TYPE_STATIC) { continue; }

			for (ContactEdge2D& ce : rb->contacts)
			{
				Contact2D* contact = ce.contact;

				if (contact->flags & Contact2D::FLAG_ISLAND || (contact->flags & Contact2D::MASK_TOUCHING_ENABLED) != Contact2D::MASK_TOUCHING_ENABLED ||
					contact->colliderA->trigger || contact->colliderB->trigger)
				{
					continue;
				}

				island.AddContact(contact);
				contact->flags |= Contact2D::FLAG_ISLAND;

				RigidBody2D* other = ce.other;

				if (other->flags & RigidBody2D::FLAG_ISLAND) { continue; }

				stack.Push(other);
				other->flags |= RigidBody2D::FLAG_ISLAND;
			}
		}

		island.Solve(step);

		for (RigidBody2D* rb : island.bodies)
		{
			if (rb->type == BODY_TYPE_STATIC) { rb->flags &= ~RigidBody2D::FLAG_ISLAND; }
		}
	}

	for (RigidBody2D& rb : *bodies)
	{
		if ((rb.flags & RigidBody2D::FLAG_ISLAND) == 0 || rb.type == BODY_TYPE_STATIC) { continue; }

		Transform2D xf1;
		xf1.rotation = rb.sweep.a0;
		xf1.position = rb.sweep.c0 - rb.sweep.localCenter * xf1.rotation;

		for (Collider2D& c : rb.colliders)
		{
			Synchronize(c, xf1, rb.transform);
		}
	}

	Broadphase::UpdatePairs(); //ContactManager
}

void Physics::SolveTOI(F32 step)
{
	Island2D island(2 * MaxTOIContacts, MaxTOIContacts);

	if (stepComplete)
	{
		for (RigidBody2D& rb : *bodies)
		{
			rb.flags &= ~RigidBody2D::FLAG_ISLAND;
			rb.sweep.alpha0 = 0.0f;
		}

		U32 i = 0;
		for (Contact2D* it = contacts; i < contactFreelist.Last(); ++i, ++it)
		{
			Contact2D& contact = *it;
			// Invalidate TOI
			contact.flags &= ~(Contact2D::FLAG_TOI | Contact2D::FLAG_ISLAND);
			contact.toiCount = 0;
			contact.toi = 1.0f;
		}
	}

	// Find TOI events and solve them.
	while (true)
	{
		// Find the first TOI.
		Contact2D* minContact = nullptr;
		F32 minAlpha = 1.0f;

		U32 i = 0;
		for (Contact2D* contact = contacts; i < contactFreelist.Last(); ++i, ++contact)
		{
			if ((contact->flags & Contact2D::FLAG_ENABLED) == 0 || contact->toiCount > MaxSubSteps) { continue; }

			F32 alpha = 1.0f;
			if (contact->flags & Contact2D::FLAG_TOI)
			{
				// This contact has a valid cached TOI.
				alpha = contact->toi;
			}
			else
			{
				Collider2D* fA = contact->colliderA;
				Collider2D* fB = contact->colliderB;

				if (fA->trigger || fB->trigger) { continue; }

				RigidBody2D* bA = fA->body;
				RigidBody2D* bB = fB->body;

				BodyType typeA = bA->type;
				BodyType typeB = bB->type;

				bool activeA = bA->flags & RigidBody2D::FLAG_AWAKE && typeA != BODY_TYPE_STATIC;
				bool activeB = bB->flags & RigidBody2D::FLAG_AWAKE && typeB != BODY_TYPE_STATIC;

				// Is at least one body active (awake and dynamic or kinematic)?
				if (activeA == false && activeB == false) { continue; }

				bool collideA = bA->flags & RigidBody2D::FLAG_BULLET || typeA != BODY_TYPE_DYNAMIC;
				bool collideB = bB->flags & RigidBody2D::FLAG_BULLET || typeB != BODY_TYPE_DYNAMIC;

				// Are these two non-bullet dynamic bodies?
				if (collideA == false && collideB == false) { continue; }

				// Compute the TOI for this contact.
				// Put the sweeps onto the same time interval.
				F32 alpha0 = bA->sweep.alpha0;

				if (bA->sweep.alpha0 < bB->sweep.alpha0)
				{
					alpha0 = bB->sweep.alpha0;
					bA->sweep.Advance(alpha0);
				}
				else if (bB->sweep.alpha0 < bA->sweep.alpha0)
				{
					alpha0 = bA->sweep.alpha0;
					bB->sweep.Advance(alpha0);
				}

				// Compute the time of impact in interval [0, minTOI]
				TOIInput input;
				input.proxyA.Set(fA);
				input.proxyB.Set(fB);
				input.sweepA = bA->sweep;
				input.sweepB = bB->sweep;
				input.tMax = 1.0f;

				TOIOutput output;
				TimeOfImpact(&output, &input);

				// Beta is the fraction of the remaining portion of the .
				F32 beta = output.t;
				if (output.state == TOIOutput::State_Touching)
				{
					alpha = Math::Min(alpha0 + (1.0f - alpha0) * beta, 1.0f);
				}
				else
				{
					alpha = 1.0f;
				}

				contact->toi = alpha;
				contact->flags |= Contact2D::FLAG_TOI;
			}

			if (alpha < minAlpha)
			{
				// This is the minimum TOI found so far.
				minContact = contact;
				minAlpha = alpha;
			}
		}

		if (minContact == NULL || 1.0f - 10.0f * Traits<F32>::Epsilon < minAlpha)
		{
			// No more TOI events. Done!
			stepComplete = true;
			break;
		}

		// Advance the bodies to the TOI.
		Collider2D* cA = minContact->colliderA;
		Collider2D* cB = minContact->colliderB;
		RigidBody2D* bA = cA->body;
		RigidBody2D* bB = cB->body;

		Sweep2D backup1 = bA->sweep;
		Sweep2D backup2 = bB->sweep;

		bA->Advance(minAlpha);
		bB->Advance(minAlpha);

		// The TOI contact likely has some new contact points.
		minContact->Update();
		minContact->flags &= ~Contact2D::FLAG_TOI;
		++minContact->toiCount;

		// Is the contact solid?
		if ((minContact->flags & Contact2D::FLAG_ENABLED) == 0 || (minContact->flags & Contact2D::FLAG_TOUCHING) == 0)
		{
			// Restore the sweeps.
			minContact->flags &= ~Contact2D::FLAG_ENABLED;
			bA->sweep = backup1;
			bB->sweep = backup2;
			bA->SynchronizeTransform();
			bB->SynchronizeTransform();
			continue;
		}

		bA->SetAwake(true);
		bB->SetAwake(true);

		// Build the island
		island.Clear();
		island.AddBody(bA);
		island.AddBody(bB);
		island.AddContact(minContact);

		bA->flags |= RigidBody2D::FLAG_ISLAND;
		bB->flags |= RigidBody2D::FLAG_ISLAND;
		minContact->flags |= Contact2D::FLAG_ISLAND;

		// Get contacts on bodyA and bodyB.
		RigidBody2D* bodies[2] = { bA, bB };
		for (U32 i = 0; i < 2; ++i)
		{
			RigidBody2D* body = bodies[i];
			if (body->type == BODY_TYPE_DYNAMIC)
			{
				for (ContactEdge2D& ce : body->contacts)
				{
					if (island.bodies.Full() || island.contacts.Full()) { break; }

					Contact2D* contact = ce.contact;

					// Has this contact already been added to the island?
					if (contact->flags & Contact2D::FLAG_ISLAND) { continue; }

					// Only add static, kinematic, or bullet bodies.
					RigidBody2D* other = ce.other;
					if (other->type == BODY_TYPE_DYNAMIC && (body->flags & RigidBody2D::FLAG_BULLET) == 0 &&
						(other->flags & RigidBody2D::FLAG_BULLET) == 0)
					{
						continue;
					}

					// Skip sensors.
					if (contact->colliderA->trigger || contact->colliderB->trigger) { continue; }

					// Tentatively advance the body to the TOI.
					Sweep2D backup = other->sweep;
					if ((other->flags & RigidBody2D::FLAG_ISLAND) == 0) { other->Advance(minAlpha); }

					// Update the contact points
					contact->Update();

					// Was the contact disabled by the user?
					if ((contact->flags & Contact2D::FLAG_ENABLED) == 0)
					{
						other->sweep = backup;
						other->SynchronizeTransform();
						continue;
					}

					// Are there contact points?
					if ((contact->flags & Contact2D::FLAG_TOUCHING) == 0)
					{
						other->sweep = backup;
						other->SynchronizeTransform();
						continue;
					}

					// Add the contact to the island
					contact->flags |= Contact2D::FLAG_ISLAND;
					island.AddContact(contact);

					// Has the other body already been added to the island?
					if (other->flags & RigidBody2D::FLAG_ISLAND) { continue; }

					// Add the other body to the island.
					other->flags |= RigidBody2D::FLAG_ISLAND;

					if (other->type != BODY_TYPE_STATIC) { other->SetAwake(true); }

					island.AddBody(other);
				}
			}
		}

		island.SolveTOI((1.0f - minAlpha) * step, bA->islandIndex, bB->islandIndex);

		// Reset island flags and synchronize broad-phase proxies.

		for (RigidBody2D* rb : island.bodies)
		{
			rb->flags &= ~RigidBody2D::FLAG_ISLAND;

			if (rb->type != BODY_TYPE_DYNAMIC) { continue; }

			rb->SynchronizeFixtures();

			// Invalidate all contact TOIs on this displaced body.
			for (ContactEdge2D& ce : rb->contacts)
			{
				ce.contact->flags &= ~(Contact2D::FLAG_TOI | Contact2D::FLAG_ISLAND);
			}
		}

		// Commit fixture proxy movements to the broad-phase so that new contacts are created.
		// Also, some contacts can be destroyed.
		Broadphase::UpdatePairs();

		stepComplete = false;
		break;
	}
}

void Physics::TimeOfImpact(TOIOutput* output, const TOIInput* input)
{
	output->state = TOIOutput::State_Unknown;
	output->t = input->tMax;

	const DistanceProxy* proxyA = &input->proxyA;
	const DistanceProxy* proxyB = &input->proxyB;

	Sweep2D sweepA = input->sweepA;
	Sweep2D sweepB = input->sweepB;

	// Large rotations can make the root finder fail, so we normalize the
	// sweep angles.
	sweepA.Normalize();
	sweepB.Normalize();

	F32 tMax = input->tMax;

	F32 totalRadius = proxyA->radius + proxyB->radius;
	F32 target = Math::Max(LinearSlop, totalRadius - 3.0f * LinearSlop);
	F32 tolerance = 0.25f * LinearSlop;

	F32 t1 = 0.0f;
	const U32 k_maxIterations = 20;	// TODO_ERIN b2::Settings
	U32 iter = 0;

	// Prepare input for distance query.
	SimplexCache cache;
	cache.count = 0;
	DistanceInput distanceInput;
	distanceInput.proxyA = input->proxyA;
	distanceInput.proxyB = input->proxyB;
	distanceInput.useRadii = false;

	// The outer loop progressively attempts to compute new separating axes.
	// This loop terminates when an axis is repeated (no progress is made).
	for (;;)
	{
		Transform2D xfA = sweepA.GetTransform(t1);
		Transform2D xfB = sweepB.GetTransform(t1);

		// Get the distance between shapes. We can also use the results
		// to get a separating axis.
		distanceInput.transformA = xfA;
		distanceInput.transformB = xfB;
		DistanceOutput distanceOutput;
		Distance(&distanceOutput, &cache, &distanceInput);

		// If the shapes are overlapped, we give up on continuous collision.
		if (distanceOutput.distance <= 0.0f)
		{
			// Failure!
			output->state = TOIOutput::State_Overlapped;
			output->t = 0.0f;
			break;
		}

		if (distanceOutput.distance < target + tolerance)
		{
			// Victory!
			output->state = TOIOutput::State_Touching;
			output->t = t1;
			break;
		}

		// Initialize the separating axis.
		SeparationFunction fcn;
		fcn.Initialize(&cache, proxyA, sweepA, proxyB, sweepB, t1);

		// Compute the TOI on the separating axis. We do this by successively
		// resolving the deepest point. This loop is bounded by the number of vertices.
		bool done = false;
		F32 t2 = tMax;
		U32 pushBackIter = 0;
		for (;;)
		{
			// Find the deepest point at t2. Store the witness point indices.
			U32 indexA, indexB;
			F32 s2 = fcn.FindMinSeparation(&indexA, &indexB, t2);

			// Is the final configuration separated?
			if (s2 > target + tolerance)
			{
				// Victory!
				output->state = TOIOutput::State_Separated;
				output->t = tMax;
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
			F32 s1 = fcn.Evaluate(indexA, indexB, t1);

			// Check for initial overlap. This might happen if the root finder
			// runs out of iterations.
			if (s1 < target - tolerance)
			{
				output->state = TOIOutput::State_Failed;
				output->t = t1;
				done = true;
				break;
			}

			// Check for touching
			if (s1 <= target + tolerance)
			{
				// Victory! t1 should hold the TOI (could be 0.0).
				output->state = TOIOutput::State_Touching;
				output->t = t1;
				done = true;
				break;
			}

			// Compute 1D root of: f(x) - target = 0
			U32 rootIterCount = 0;
			F32 a1 = t1, a2 = t2;
			for (;;)
			{
				// Use a mix of the secant rule and bisection.
				F32 t;
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

				F32 s = fcn.Evaluate(indexA, indexB, t);

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
			output->state = TOIOutput::State_Failed;
			output->t = t1;
			break;
		}
	}
}

void Physics::DetectCollision()
{
	U32 i = 0;
	for (Contact2D* it = contacts; i < contactFreelist.Last(); ++i, ++it)
	{
		Contact2D& contact = *it;

		if (contact.valid == false) { continue; }

		Collider2D* colliderA = contact.colliderA;
		Collider2D* colliderB = contact.colliderB;
		RigidBody2D* rbA = colliderA->body;
		RigidBody2D* rbB = colliderB->body;

		if (!rbA->ShouldCollide(rbB) || (colliderA->layers & colliderB->layers) == 0)
		{
			contactFreelist.Release(i);
			contact.valid = false;
			contact.index = U32_MAX;
			continue;
		}

		bool activeA = (rbA->flags & RigidBody2D::FLAG_AWAKE) && rbA->type != BODY_TYPE_STATIC;
		bool activeB = (rbA->flags & RigidBody2D::FLAG_AWAKE) && rbB->type != BODY_TYPE_STATIC;

		// At least one body must be awake and it must be dynamic or kinematic.
		if (activeA == false && activeB == false) { continue; }

		U32 proxyIdA = colliderA->proxy.proxyId;
		U32 proxyIdB = colliderB->proxy.proxyId;
		bool overlap = Broadphase::TestOverlap(proxyIdA, proxyIdB);

		// Here we destroy contacts that cease to overlap in the broad-phase.
		if (overlap == false)
		{
			contactFreelist.Release(i);
			contact.valid = false;
			contact.index = U32_MAX;
			continue;
		}

		// The contact persists.
		contact.Update();
	}
}

void Physics::Synchronize(Collider2D& collider, const Transform2D& transformA, const Transform2D& transformB)
{
	AABB aabbA = collider.ComputeAABB(transformA);
	AABB aabbB = collider.ComputeAABB(transformB);

	collider.proxy.aabb.Combine(aabbA, aabbB);

	Vector2 displacement = transformB.position - transformA.position;

	Broadphase::MoveProxy(collider.proxy.proxyId, collider.proxy.aabb, displacement);
}

void Physics::AddPair(ColliderProxy* proxyA, ColliderProxy* proxyB)
{
	Collider2D* colliderA = proxyA->collider;
	Collider2D* colliderB = proxyB->collider;

	RigidBody2D* bodyA = colliderA->body;
	RigidBody2D* bodyB = colliderB->body;

	if (bodyA == bodyB) { return; }

	//TODO: Maybe use a hashmap, there could be a lot of contacts

	// Does a contact already exist?
	for (ContactEdge2D& edge : bodyB->contacts)
	{
		if (edge.other == bodyA) { return; }
	}

	// Does a joint override collision? Is at least one body dynamic?
	if (bodyB->ShouldCollide(bodyA) == false) { return; }

	// Check user filtering.
	if ((colliderA->layers & colliderB->layers) == 0) { return; }

	if (contactFreelist.Full())
	{
		Memory::Reallocate(&contacts, contactCapacity + 1, contactCapacity);
		contactFreelist.Resize((U32)contactCapacity);
	}

	U32 index = contactFreelist.GetFree();
	Contact2D* c = &contacts[index];

	c->colliderA = colliderA;
	c->colliderB = colliderB;
	c->index = index;
	c->valid = true;

	bodyA->contacts.Push({ bodyB, c });
	bodyB->contacts.Push({ bodyA, c });

	// Wake up the bodies
	if (colliderA->trigger == false && colliderB->trigger == false)
	{
		bodyA->SetAwake(true);
		bodyB->SetAwake(true);
	}
}

void Physics::Distance(DistanceOutput* output, SimplexCache* cache, const DistanceInput* input)
{
	const DistanceProxy* proxyA = &input->proxyA;
	const DistanceProxy* proxyB = &input->proxyB;

	Transform2D transformA = input->transformA;
	Transform2D transformB = input->transformB;

	// Initialize the simplex.
	Simplex simplex;
	simplex.ReadCache(cache, proxyA, transformA, proxyB, transformB);

	// Get simplex vertices as an array.
	SimplexVertex* vertices = &simplex.v1;
	const U32 k_maxIters = 20;

	// These store the vertices of the last simplex so that we
	// can check for duplicates and prevent cycling.
	U32 saveA[3], saveB[3];
	U32 saveCount = 0;

	F32 distanceSqr1 = F32_MAX;
	F32 distanceSqr2 = distanceSqr1;

	// Main iteration loop.
	U32 iter = 0;
	while (iter < k_maxIters)
	{
		// Copy simplex so we can identify duplicates.
		saveCount = simplex.count;
		for (U32 i = 0; i < saveCount; ++i)
		{
			saveA[i] = vertices[i].indexA;
			saveB[i] = vertices[i].indexB;
		}

		switch (simplex.count)
		{
		case 1: break;
		case 2: { simplex.Solve2(); } break;
		case 3: { simplex.Solve3(); } break;
		default: break;
		}

		// If we have 3 points, then the origin is in the corresponding triangle.
		if (simplex.count == 3) { break; }

		// Compute closest point.
		Vector2 p = simplex.GetClosestPoint();
		distanceSqr2 = p.SqrMagnitude();

		// Ensure progress
		distanceSqr1 = distanceSqr2;

		// Get search direction.
		Vector2 d = simplex.GetSearchDirection();

		// Ensure the search direction is numerically fit.
		if (d.SqrMagnitude() < Traits<F32>::Epsilon * Traits<F32>::Epsilon) { break; }

		// Compute a tentative new simplex vertex using support points.
		SimplexVertex* vertex = vertices + simplex.count;
		vertex->indexA = proxyA->GetSupport(-d ^ transformA.rotation);
		vertex->wA = proxyA->vertices[vertex->indexA] * transformA;
		Vector2 wBLocal;
		vertex->indexB = proxyB->GetSupport(d ^ transformB.rotation);
		vertex->wB = proxyB->vertices[vertex->indexB] * transformB;
		vertex->w = vertex->wB - vertex->wA;

		// Iteration count is equated to the number of support point calls.
		++iter;

		// Check for duplicate support points. This is the main termination criteria.
		bool duplicate = false;
		for (U32 i = 0; i < saveCount; ++i)
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

	// Prepare output.
	simplex.GetWitnessPoints(&output->pointA, &output->pointB);
	output->distance = (output->pointA - output->pointB).Magnitude();
	output->iterations = iter;

	// Cache the simplex.
	simplex.WriteCache(cache);

	// Apply radii if requested.
	if (input->useRadii)
	{
		F32 rA = proxyA->radius;
		F32 rB = proxyB->radius;

		if (output->distance > rA + rB && output->distance > Traits<F32>::Epsilon)
		{
			// Shapes are still no overlapped.
			// Move the witness points to the outer surface.
			output->distance -= rA + rB;
			Vector2 normal = output->pointB - output->pointA;
			normal.Normalize();
			output->pointA += rA * normal;
			output->pointB -= rB * normal;
		}
		else
		{
			// Shapes are overlapped when radii are considered.
			// Move the witness points to the middle.
			Vector2 p = 0.5f * (output->pointA + output->pointB);
			output->pointA = p;
			output->pointB = p;
			output->distance = 0.0f;
		}
	}
}

bool Physics::TestOverlap(const Collider2D* shapeA, const Collider2D* shapeB,
	const Transform2D& xfA, const Transform2D& xfB)
{
	DistanceInput input;
	input.proxyA.Set(shapeA);
	input.proxyB.Set(shapeB);
	input.transformA = xfA;
	input.transformB = xfB;
	input.useRadii = true;

	SimplexCache cache;
	cache.count = 0;

	DistanceOutput output;

	Distance(&output, &cache, &input);

	return output.distance < 10.0f * Traits<F32>::Epsilon;
}