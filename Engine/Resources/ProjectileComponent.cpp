#include "ProjectileComponent.hpp"

#include "Scene.hpp"

#include "Core/Time.hpp"

Vector<Projectile> Projectile::components(10000, {});
Freelist Projectile::freeComponents(10000);
bool Projectile::initialized = false;

bool Projectile::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Projectile::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Projectile> Projectile::AddTo(const EntityRef& entity, const Vector2& velocity, F32 acceleration, F32 gravity)
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
	projectile.hit = false;

	return { entity.EntityId(), instanceId };
}

void Projectile::RemoveFrom(const EntityRef& entity)
{
	ComponentRef<Projectile> projectile = Get(entity);
	projectile->OnHit.Destroy();

	Destroy(*projectile);
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
			projectile->OnHit(EntityRef{ projectile->entityIndex, (U32)(projectile - components.Data()) });
			continue;
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
	F32 dt = Math::Min((F32)Time::DeltaTime(), 0.25f);

	Vector2 dir = velocity.Normalized();

	velocity += (dir * acceleration - Vector2{ 0.0f, gravity }) * dt;
	Vector2 frameVelocity = velocity * dt;
	Vector2 target = position + frameVelocity;
	Collision collision;

	if (collision = Physics::CheckCollision(collider + position + frameVelocity))
	{
		hit = true;
	}
	else
	{
		position = target;
	}
}