#include "Enemy.hpp"

#include "TimeSlip.hpp"

#include <Memory/Memory.hpp>
#include <Resources/Resources.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Physics/Physics.hpp>

Enemy::Enemy(const Vector2& position, EnemyAI aiType, bool aggressive) : Entity(position), aiType{ aiType }, target{ nullptr }, aggressive{ aggressive }
{
	
}

Enemy::~Enemy()
{
	Destroy();
}

void Enemy::Destroy()
{
	target = nullptr;
}

void* Enemy::operator new(U64 size) { return Memory::Allocate(sizeof(Enemy), MEMORY_TAG_GAME); }
void Enemy::operator delete(void* ptr) { Memory::Free(ptr, sizeof(Enemy), MEMORY_TAG_GAME); }

void Enemy::Update()
{
	if (aggressive && !target)
	{
		target = TimeSlip::GetTarget(gameObject->transform);
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
	if (target)
	{
		Vector2 dir = target->Position() - gameObject->transform->Position();
		F32 dist = dir.Magnitude();

		if (dist > 10.0f)
		{
			startChase = false;
			target = nullptr;
		}
		else if (dist < 1.0f)
		{
			startChase = false;
			//TODO: Attack
		}
		else
		{
			Vector2 move{ (F32)Math::Sign(dir.x), 0.0f };
			move *= (F32)(Time::DeltaTime() * 5.0f);

			if (((gameObject->transform->Position() - lastPos).IsZero() && startChase) && gameObject->physics->Grounded())
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