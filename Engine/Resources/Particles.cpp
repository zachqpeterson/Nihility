#include "Particles.hpp"

#include "Entity.hpp"
#include "World.hpp"

#include "Resources/SpriteComponent.hpp"
#include "Resources/ProjectileComponent.hpp"

bool OnHit(const EntityRef& entity, bool hitVertical)
{
	ComponentRef<Projectile> p = Projectile::GetRef(entity);

	if (hitVertical) { p->velocity.y *= -0.25f; }
	else { p->velocity.x *= -0.25f; }

	return false;
}

bool OnUpdate(const EntityRef& entity)
{
	ComponentRef<Projectile> p = Projectile::GetRef(entity);
	ComponentRef<Sprite> s = Sprite::GetRef(entity);
	s->SetColor({1.0f, 1.0f, 1.0f, p->timer });

	return false;
}

bool OnExpire(const EntityRef& entity)
{
	Sprite::RemoveFrom(entity);
	Projectile::RemoveFrom(entity);
	World::DestroyEntity(entity);

	return false;
}

void Particles::Spawn(const Vector2& position, ResourceRef<Texture> texture)
{
	for (U32 i = 0; i < 5; i++)
	{
		Quaternion2 rot = Quaternion2::Random();
		Quaternion2 dir = Quaternion2::Random();

		EntityRef entity = World::CreateEntity(position, 0.25f, rot);

		ComponentRef<Sprite> s = Sprite::AddTo(entity, texture);
		ComponentRef<Projectile> p = Projectile::AddTo(entity, Vector2{ dir.x, dir.y } * 2.0f, 1.0f, 0.0f, 50.0f);

		if (s && p)
		{
			p->OnHit += OnHit;
			p->OnUpdate += OnUpdate;
			p->OnExpire += OnExpire;
		}
	}
}