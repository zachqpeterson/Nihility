#include "Physics.hpp"

#include "Core\Time.hpp"
#include "Core\Logger.hpp"
#include "Resources\Scene.hpp"

void RigidBody2D::Update(Scene* scene)
{

}

void RigidBody2D::Load(Scene* scene)
{

}

Scene* Physics::scene;
Vector<RigidBody2D>* Physics::bodies;
Vector<Manifold2D> Physics::manifolds;
Vector3 Physics::gravity;

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

void Physics::Update(F64 step)
{
	manifolds.Clear();

	for (RigidBody2D& rb : *bodies)
	{
		if (rb.simulated)
		{
			rb.acceleration = 0.0f;

			rb.acceleration += gravity.xy(); //TODO: * step?

			rb.velocity += rb.acceleration * (F32)step;
			rb.position += rb.velocity * (F32)step;
		}
	}

	//TODO: Broadphase Culling

	for (RigidBody2D* it0 = bodies->begin(); it0 != bodies->end(); ++it0)
	{
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

	for (Manifold2D& manifold : manifolds)
	{
		ResolveCollision(manifold);
	}


	//F64 a = 1.4;
	//F64 b = 0.7;
	//F64 c = 2.1;
	//F32 v = a * b * c;
	//Vector3 i0 = v * Vector3{ b * b + c * c, a * a + c * c, a * a + b * b } / 12.0f;
	//
	//RigidBody rb;
	//rb.invI0 = 1.0 / i0;
	//
	//Matrix3 r = rb.rotation.ToMatrix3();
	//
	//Vector3 omega = r * rb.invI0 * r.Transpose() * rb.angularMomentum;
	//
	//rb.position += rb.velocity * (F32)Time::DeltaTime();
	//rb.rotation = Quaternion3{ omega * (F32)(step / 2.0) } * rb.rotation;
}

bool Physics::DetectCollision(Manifold2D& manifold)
{
	const RigidBody2D& rb0 = *manifold.rb0;
	const RigidBody2D& rb1 = *manifold.rb1;
	
	if ((rb0.layers & rb1.layers) == 0) { return false; }

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
					if (overlapX > overlapY)
					{
						if (n.x < 0.0f) { manifold.normal = Vector2Left; }
						else { manifold.normal = Vector2Right; }
						manifold.penetration = overlapX;
					}
					else
					{
						if (n.y < 0.0f) { manifold.normal = Vector2Down; }
						else { manifold.normal = Vector2Up; }
						manifold.penetration = overlapY;
					}

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
			F32 r = rb0.collider.circle.radius + rb1.collider.circle.radius;
			r *= r;

			F32 lengthSqr = (rb0.position + rb1.position).SqrMagnitude();

			if (lengthSqr > r) { return false; }

			F32 d = Math::Sqrt(lengthSqr);

			if (!Math::IsZero(d))
			{
				manifold.penetration = r - d;
				manifold.normal = n / d;
				return true;
			}

			manifold.penetration = rb0.collider.circle.radius;
			manifold.normal = Vector2Right;
			return true;
		} break;
		}
	} break;
	}

	return false;
}

void Physics::ResolveCollision(Manifold2D& manifold)
{
	// Calculate relative velocity
	Vector2 rv = manifold.rb0->velocity - manifold.rb1->velocity;

	// Calculate relative velocity in terms of the normal direction
	F32 velAlongNormal = rv.Dot(manifold.normal);

	// Do not resolve if velocities are separating
	if (velAlongNormal > 0.0f) { return; }

	// Calculate restitution
	F32 e = Math::Min(manifold.rb0->restitution, manifold.rb1->restitution);

	// Calculate impulse scalar
	F32 j = -(1.0f + e) * velAlongNormal;

	j /= manifold.rb0->invMass + manifold.rb1->invMass;

	// Apply impulse
	Vector2 impulse = manifold.normal * j;

	manifold.rb0->velocity -= impulse * manifold.rb0->invMass;
	manifold.rb1->velocity += impulse * manifold.rb1->invMass;
}

void Physics::CorrectPosition(Manifold2D& manifold)
{
	const F32 percent = 0.2f;	//0.2 to 0.8
	const F32 slop = 0.01f;		//0.01 to 0.1

	Vector2 correction = manifold.normal * (Math::Max(manifold.penetration - slop, 0.0f) / (manifold.rb0->invMass + manifold.rb1->invMass)) * percent;

	manifold.rb0->position -= correction * manifold.rb0->invMass;
	manifold.rb1->position += correction * manifold.rb1->invMass;
}