#include "Physics.hpp"

#include "Core\Time.hpp"
#include "Core\Logger.hpp"

Scene* Physics::scene;
Vector<RigidBody2D>* Physics::bodies;
Vector<Manifold2D> Physics::manifolds;
Vector3 Physics::gravity = Vector3Down * 9.8f;
Vector2 Physics::wind = Vector2Zero;
F32 Physics::fluidDensity = 1.3; //density of air 1.293 kg m-3

bool Physics::Initialize()
{
	Logger::Trace("Initializing Physics...");

	return true;
}

void Physics::Shutdown()
{
	Logger::Trace("Shutting Down Physics...");
}

void Physics::SetScene(Scene* scene_)
{
	scene = scene_;

	scene->RegisterComponent<RigidBody2D>();
	bodies = scene->GetComponentPool<RigidBody2D>();
}

Collider2D Physics::CreateAABBCollider(F32 halfWidth, F32 halfHeight)
{
	return { COLLIDER_TYPE_AABB, halfWidth, halfHeight };
}

Collider2D Physics::CreateCircleCollider(F32 radius)
{
	return { COLLIDER_TYPE_CIRCLE, radius };
}

void Physics::IntegrateForces(F32 step)
{
	for (RigidBody2D& rb : *bodies)
	{
		if (rb.simulated)
		{
			rb.acceleration += gravity.xy();

			rb.velocity += rb.acceleration * rb.invMass * step * 0.5f;
			rb.angularMomentum += rb.torque * rb.invInertia * step * 0.5f;
			//rb.velocity -= 0.5f * fluidDensity * (rb.velocity + wind) * rb.dragCoefficient * step;
		}
	}
}

void Physics::IntegrateVelocity(F32 step)
{
	for (RigidBody2D& rb : *bodies)
	{
		if (rb.simulated)
		{
			rb.position += rb.velocity * step;
			rb.rotation += rb.angularMomentum * step;
		}
	}

	IntegrateForces(step);
}

void Physics::Update(F32 step)
{
	manifolds.Clear();

	//TODO: Broadphase Culling

	for (RigidBody2D* it0 = bodies->begin(); it0 != bodies->end(); ++it0)
	{
		if (it0->simulated) { it0->acceleration = Vector2Zero; it0->torque = 0.0f; }

		for (RigidBody2D* it1 = it0 + 1; it1 != bodies->end(); ++it1)
		{
			Manifold2D manifold{};
			manifold.rb0 = it0;
			manifold.rb1 = it1;

			if (DetectCollision(manifold))
			{
				bool handled = false;

				if (manifold.rb0->event)
				{
					if (manifold.rb0->trigger) { manifold.rb0->event(manifold, PHYSICS_EVENT_ON_TRIGGER_ENTER); handled = true; }
					else { handled |= manifold.rb0->event(manifold, PHYSICS_EVENT_ON_COLLISION); }
				}

				if (manifold.rb1->event)
				{
					if (manifold.rb1->trigger) { manifold.rb1->event(manifold, PHYSICS_EVENT_ON_TRIGGER_ENTER); handled = true; }
					else { handled |= manifold.rb1->event(manifold, PHYSICS_EVENT_ON_COLLISION); }
				}

				if (!handled) { manifolds.Push(manifold); }
			}
		}
	}

	IntegrateForces(step);

	for (Manifold2D& manifold : manifolds)
	{
		ResolveCollision(manifold, step);
	}

	IntegrateVelocity(step);

	for (Manifold2D& manifold : manifolds)
	{
		CorrectPosition(manifold);
	}
}

bool Physics::DetectCollision(Manifold2D& manifold)
{
	const RigidBody2D& rb0 = *manifold.rb0;
	const RigidBody2D& rb1 = *manifold.rb1;

	if ((rb0.layers & rb1.layers) == 0 || !(rb0.simulated || rb1.simulated)) { return false; }

	Vector2 n = rb1.position - rb0.position;

	switch (rb0.collider.type)
	{
	case COLLIDER_TYPE_AABB: {
		switch (rb1.collider.type)
		{
		case COLLIDER_TYPE_AABB: {
			Collider2D::AABB aabb0 = rb0.collider.aabb;
			Collider2D::AABB aabb1 = rb1.collider.aabb;

			F32 overlapX = aabb0.halfWidth + aabb1.halfWidth - Math::Abs(n.x);

			if (overlapX > 0.0f)
			{
				F32 overlapY = aabb0.halfHeight + aabb1.halfHeight - Math::Abs(n.y);

				if (overlapY > 0.0f)
				{
					manifold.contactCount = 1;

					if (overlapX < overlapY)
					{
						if (n.x < 0.0f)
						{
							manifold.normal = Vector2Left;
							manifold.contacts[0] = Vector2{ aabb0.halfWidth, 0.0f };
						}
						else
						{
							manifold.normal = Vector2Right;
							manifold.contacts[0] = Vector2{ -aabb0.halfWidth, 0.0f };
						}

						manifold.penetration = overlapX;
					}
					else
					{
						if (n.y < 0.0f)
						{
							manifold.normal = Vector2Down;
							manifold.contacts[0] = Vector2{ 0.0f, aabb1.halfHeight };
						}
						else
						{
							manifold.normal = Vector2Up;
							manifold.contacts[0] = Vector2{ 0.0f, -aabb1.halfHeight };
						}

						manifold.penetration = overlapY;
					}

					Vector2 ra = manifold.contacts[0] - rb0.position;
					Vector2 rb = manifold.contacts[0] - rb1.position;
					manifold.relativeVelocity = rb1.velocity + Math::Cross(rb1.angularMomentum, rb) - rb0.velocity - Math::Cross(rb0.angularMomentum, ra);

					if (manifold.relativeVelocity.Dot(manifold.normal) > 0.0f && manifold.penetration < 0.01f) { return false; }

					return true;
				}
			}

			return false;
		} break;
		case COLLIDER_TYPE_CIRCLE: {
			Collider2D::AABB aabb = rb0.collider.aabb;

			Vector2 closest = n;

			closest.x = Math::Clamp(-aabb.halfWidth, aabb.halfWidth, closest.x);
			closest.y = Math::Clamp(-aabb.halfHeight, aabb.halfHeight, closest.y);

			bool inside = false;

			if (n == closest)
			{
				inside = true;

				if (Math::Abs(n.x) > Math::Abs(n.y))
				{
					if (closest.x > 0.0f) { closest.x = aabb.halfWidth; }
					else { closest.x = -aabb.halfWidth; }
				}
			}
			else
			{
				if (closest.y > 0.0f) { closest.y = aabb.halfHeight; }
				else { closest.y = -aabb.halfHeight; }
			}

			Vector2 normal = rb1.position - closest;
			F32 d = normal.SqrMagnitude();
			F32 r = rb1.collider.circle.radius;

			if (d > r * r && !inside) { return false; }

			d = Math::Sqrt(d);

			manifold.penetration = r - d;

			if (inside) { manifold.normal = -normal / d; }
			else { manifold.normal = normal / d; }

			return true;
		} break;
		}
	} break;
	case COLLIDER_TYPE_CIRCLE: {
		switch (rb1.collider.type)
		{
		case COLLIDER_TYPE_AABB: {
			Collider2D::AABB aabb = rb1.collider.aabb;

			Vector2 closest = n;

			closest.x = Math::Clamp(-aabb.halfWidth, aabb.halfWidth, closest.x);
			closest.y = Math::Clamp(-aabb.halfHeight, aabb.halfHeight, closest.y);

			bool inside = false;

			if (n == closest)
			{
				inside = true;

				if (Math::Abs(n.x) > Math::Abs(n.y))
				{
					if (closest.x > 0.0f) { closest.x = aabb.halfWidth; }
					else { closest.x = -aabb.halfWidth; }
				}
			}
			else
			{
				if (closest.y > 0.0f) { closest.y = aabb.halfHeight; }
				else { closest.y = -aabb.halfHeight; }
			}

			Vector2 normal = rb0.position - closest;
			F32 d = normal.SqrMagnitude();
			F32 r = rb0.collider.circle.radius;

			if (d > r * r && !inside) { return false; }

			d = Math::Sqrt(d);

			manifold.penetration = r - d;

			if (inside) { manifold.normal = -normal / d; }
			else { manifold.normal = normal / d; }

			return true;
		} break;
		case COLLIDER_TYPE_CIRCLE: {
			Collider2D::Circle c0 = rb0.collider.circle;
			Collider2D::Circle c1 = rb1.collider.circle;

			Vector2 normal = rb1.position - rb0.position;

			F32 distSqr = normal.SqrMagnitude();

			F32 radius = c0.radius + c1.radius;

			if (distSqr > radius * radius) { return false; }

			F32 dist = Math::Sqrt(distSqr);

			if (Math::IsZero(dist))
			{
				manifold.penetration = c0.radius;
				manifold.normal = Vector2Left;
				manifold.contacts[0] = rb0.position;
				manifold.contactCount = 1;
			}
			else
			{
				manifold.penetration = radius - dist;
				manifold.normal = normal / dist;
				manifold.contacts[0] = manifold.normal * c0.radius + rb0.position;
				manifold.contactCount = 1;
			}

			Vector2 ra = manifold.contacts[0] - rb0.position;
			Vector2 rb = manifold.contacts[0] - rb1.position;
			manifold.relativeVelocity = rb1.velocity + Math::Cross(rb1.angularMomentum, rb) - rb0.velocity - Math::Cross(rb0.angularMomentum, ra);

			if (manifold.relativeVelocity.Dot(manifold.normal) > 0.0f && manifold.penetration < 0.01f) { return false; }

			return true;
		} break;
		}
	} break;
	}

	return false;
}

void Physics::ResolveCollision(Manifold2D& manifold, F32 step)
{
	RigidBody2D& rb0 = *manifold.rb0;
	RigidBody2D& rb1 = *manifold.rb1;

	F32 restitution = Math::Min(rb0.restitution, rb1.restitution);
	F32 dynamicFriction = Math::Sqrt(rb0.dynamicFriction * rb1.dynamicFriction);
	F32 staticFriction = Math::Sqrt(rb0.staticFriction * rb1.staticFriction);

	for (U8 i = 0; i < manifold.contactCount; ++i)
	{
		Vector2 ra = manifold.contacts[i] - rb0.position;
		Vector2 rb = manifold.contacts[i] - rb1.position;

		if (manifold.relativeVelocity.SqrMagnitude() < (gravity * step).SqrMagnitude() + Traits<F32>::Epsilon) { restitution = 0.0f; }

		// Relative velocity along the normal
		F32 contactVel = manifold.relativeVelocity.Dot(manifold.normal);

		F32 raCrossN = ra.Cross(manifold.normal);
		F32 rbCrossN = rb.Cross(manifold.normal);
		F32 invMassSum = rb0.invMass + rb1.invMass + raCrossN * raCrossN * rb0.invInertia + rbCrossN * rbCrossN * rb1.invInertia;

		// Calculate impulse scalar
		F32 j = -(1.0f + restitution) * contactVel;
		j /= invMassSum;
		j /= (F32)manifold.contactCount;

		// Apply impulse
		Vector2 impulse = manifold.normal * j;
		rb0.ApplyImpulse(-impulse, ra);
		rb1.ApplyImpulse(impulse, rb);

		Vector2 t = manifold.relativeVelocity - (manifold.normal * manifold.relativeVelocity.Dot(manifold.normal));
		t.Normalize();

		// j tangent magnitude
		F32 jt = -manifold.relativeVelocity.Dot(t);
		jt /= invMassSum;
		jt /= (F32)manifold.contactCount;

		// Don't apply tiny friction impulses
		if (Math::IsZero(jt)) { break; }

		// Coulumb's law
		Vector2 tangentImpulse;
		if (Math::Abs(jt) < j * staticFriction) { tangentImpulse = t * jt; }
		else { tangentImpulse = t * -j * dynamicFriction; }

		// Apply friction impulse
		rb0.ApplyImpulse(-tangentImpulse, ra);
		rb1.ApplyImpulse(tangentImpulse, rb);
	}
}

void Physics::CorrectPosition(Manifold2D& manifold)
{
	const F32 percent = 0.2f;	//0.2 to 0.8
	const F32 slop = 0.01f;		//0.01 to 0.1

	Vector2 correction = manifold.normal * (Math::Max(manifold.penetration - slop, 0.0f) / (manifold.rb0->invMass + manifold.rb1->invMass)) * percent;

	manifold.rb0->position -= correction * manifold.rb0->invMass;
	manifold.rb1->position += correction * manifold.rb1->invMass;
}