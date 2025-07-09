#include "ProjectileComponent.hpp"

#include "World.hpp"

#include "Core/Time.hpp"

Vector<Projectile> Projectile::components(10000, {});
Freelist Projectile::freeComponents(10000);
bool Projectile::initialized = false;

bool Projectile::Initialize()
{
	if (!initialized)
	{
		World::UpdateFns += Update;
		World::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Projectile::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Projectile> Projectile::AddTo(const EntityRef& entity, const Vector2& velocity, F32 duration, F32 acceleration, F32 gravity)
{
	if (freeComponents.Full()) { Logger::Error("Max Projectile Instances Reached!"); return nullptr; }

	U32 instanceId;
	Projectile& projectile = Create(instanceId);
	projectile.entityIndex = entity.EntityId();
	projectile.position = entity->position;
	projectile.velocity = velocity;
	projectile.collider = { entity->scale, -entity->scale };
	projectile.acceleration = acceleration;
	projectile.gravity = gravity;
	projectile.timer = duration;
	projectile.expire = duration > 0.0f;
	projectile.hit = false;

	return { entity.EntityId(), instanceId };
}

void Projectile::RemoveFrom(const EntityRef& entity)
{
	ComponentRef<Projectile> projectile = GetRef(entity);
	if (projectile)
	{
		projectile->OnHit.Destroy();
		projectile->OnUpdate.Destroy();
		projectile->OnExpire.Destroy();

		Destroy(*projectile);
	}
}

bool Projectile::Update(Camera& camera, Vector<Entity>& entities)
{
	for (Projectile* projectile = components.begin(); projectile != components.end(); ++projectile)
	{
		if (projectile->entityIndex == U32_MAX) { continue; }
		Entity& entity = entities[projectile->entityIndex];
		projectile->Simulate();

		if (projectile->hit && projectile->OnHit)
		{
			projectile->OnHit({ projectile->entityIndex }, projectile->hitVertical);
			projectile->hit = false;
		}

		if (projectile->OnUpdate)
		{
			projectile->OnUpdate({ projectile->entityIndex });
		}

		if (projectile->OnExpire && projectile->timer <= 0.0f && projectile->expire)
		{
			projectile->expire = false;
			projectile->OnExpire({ projectile->entityIndex });
		}

		entity.position = projectile->position;
	}

	return false;
}

bool Projectile::Render(CommandBuffer commandBuffer)
{
	return false;
}

void Projectile::Simulate()
{
	F32 dt = (F32)Time::DeltaTimeStable();

	timer -= dt;
	Vector2 dir = velocity.Normalized();

	velocity += (dir * acceleration - Vector2{ 0.0f, gravity }) * dt;
	Vector2 frameVelocity = velocity * dt;
	Vector2 target = position + frameVelocity;
	Collision collision;

	if (collision = Physics::CheckCollision(collider + position + Vector2{ frameVelocity.x, 0.0f }))
	{
		hit = true;
		hitVertical = false;
	}
	else
	{
		position.x = target.x;
	}

	if (collision = Physics::CheckCollision(collider + position + Vector2{ 0.0f, frameVelocity.y }))
	{
		hit = true;
		hitVertical = true;
	}
	else
	{
		position.y = target.y;
	}
}