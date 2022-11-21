#include "Enemy.hpp"

#include "TimeSlip.hpp"

#include <Memory/Memory.hpp>
#include <Resources/Resources.hpp>
#include <Resources/UI.hpp>
#include <Resources/Animations.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Physics/Physics.hpp>
#include <Audio/Audio.hpp>

Enemy::Enemy(const EntityConfig& config, EnemyAI aiType, bool aggressive) : Entity(config, false),
aiType{ aiType }, target{ nullptr }, aggressive{ aggressive }, startChase{ false }, attackCooldown{ 0.0f }
{

}

Enemy::~Enemy()
{
	Destroy();
}

void Enemy::Destroy()
{
	Entity::Destroy();
	target = nullptr;
}

bool Enemy::Death()
{
	//TODO: Drop Items
	Audio::PlaySFX("Death.wav", 1.0f, 0.75f);

	Destroy();

	return true;
}

void* Enemy::operator new(U64 size) { return Memory::Allocate(sizeof(Enemy), MEMORY_TAG_GAME); }
void Enemy::operator delete(void* ptr) { Memory::Free(ptr, sizeof(Enemy), MEMORY_TAG_GAME); }

void Enemy::Update()
{
	health += healthRegen * (F32)Time::DeltaTime();

	if (aggressive && !target)
	{
		target = TimeSlip::GetTarget(gameObject->transform, 40.0f); //TODO: Chase range
	}

	switch (aiType)
	{
	case ENEMY_AI_BASIC: { BasicAI(); }  break;
	case ENEMY_AI_FLYING: { FlyingAI(); } break;
	case ENEMY_AI_RANGED: { RangedAI(); } break;
	default: break;
	}

	lastPos = gameObject->transform->Position();
}

void Enemy::BasicAI()
{
	attackCooldown -= (F32)Time::DeltaTime();

	if (target)
	{
		const Vector2& playerPos = target->Position();
		const Vector2& pos = gameObject->transform->Position();

		F32 dist = Math::Abs(playerPos.x - pos.x);

		if (dist > 40.0f) //TODO: Chase range
		{
			startChase = false;
			target = nullptr;
			if (animation->y < 2) { Animations::SetAnimation(animation, 0, true); }
			else { animation->nextAnimation = 0; }
		}
		else if (dist < 1.0f) //TODO: Attack range
		{
			startChase = false;
			if (animation->y < 2) { Animations::SetAnimation(animation, 0, true); }
			else { animation->nextAnimation = 0; }

			if (attackCooldown <= 0.0f)
			{
				attackCooldown = 0.25f;

				Damage damage{10.0f, 0.0f, 0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 0.25f};

				Vector4 area{};

				Vector2 position = gameObject->transform->Position();
				Vector2 extents = gameObject->physics->Collider()->box.Extents();

				if (facing)
				{
					area.x = position.x + extents.x + 0.01f;
					area.z = area.x + 1.0f;
					area.y = position.y - extents.y;
					area.w = area.y + 2.0f;
				}
				else
				{
					area.z = position.x - extents.x - 0.01f;
					area.x = area.x - 1.0f;
					area.y = position.y - extents.y;
					area.w = area.y + 2.0f;
				}

				TimeSlip::Attack(damage, area); //TODO: damage mask
				Animations::SetAnimation(animation, 2);
			}
		}
		else
		{
			Vector2 move{ (F32)Math::Sign(playerPos.x - pos.x), 0.0f };
			move *= (F32)(Time::DeltaTime() * 5.0f);

			facing = move.x > 0.0f;
			animation->direction = facing;
			if (animation->y < 2) { Animations::SetAnimation(animation, 1, true); }
			else { animation->nextAnimation = 1; }

			if (((pos - lastPos).IsZero() && startChase && pos.y > playerPos.y) && gameObject->physics->Grounded())
			{
				gameObject->physics->ApplyForce({ 0.0f, -1.0f });
			}

			gameObject->physics->Translate(move);

			startChase = true;
		}
	}
}

void Enemy::FlyingAI()
{

}

void Enemy::RangedAI()
{

}

void Enemy::DamageResponse()
{
	if (health > 0) { Audio::PlaySFX("Hurt.wav", 1.0f, Math::RandomF() * 0.25 + 0.5f); }
}