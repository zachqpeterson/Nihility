#include "Player.hpp"

#include "TimeSlip.hpp"

#include <Math/Math.hpp>
#include <Memory/Memory.hpp>
#include <Resources/Resources.hpp>
#include <Resources/UI.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Physics/Physics.hpp>

Player::Player(const EntityConfig& config) : Entity(config, true),
alive{ true }, deathTimer{ 0.0f }, spawnPoint{ config.position }, attackCooldown{ 0.0f }
{
	UIElementConfig barConfig{};
	barConfig.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	barConfig.enabled = true;
	barConfig.ignore = true;
	barConfig.parent = nullptr;
	barConfig.position = { 0.5f, 0.05f };
	barConfig.scale = { 0.33f, 0.05f };
	barConfig.scene = RendererFrontend::CurrentScene();

	healthBar = UI::GenerateBar(barConfig, { 1.0f, 0.0f, 0.0f, 1.0f }, 1.0f);
}

Player::~Player()
{
	Destroy();
}

void Player::Destroy()
{
	Entity::Destroy();
}

bool Player::Death()
{
	alive = false;
	deathTimer = 3.0f;

	RendererFrontend::UndrawGameObject(gameObject);
	ignore = true;

	return false;
}

void* Player::operator new(U64 size) { return Memory::Allocate(sizeof(Player), MEMORY_TAG_GAME); }
void Player::operator delete(void* ptr) { Memory::Free(ptr, sizeof(Player), MEMORY_TAG_GAME); }

void Player::Update()
{
	attackCooldown -= (F32)Time::DeltaTime();

	if (alive)
	{
		Vector2 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), 0.0f };
		move *= (F32)(Time::DeltaTime() * 10.0f);

		if (!Math::Zero(move.x))
		{
			facing = move.x > 0.0f;
		}

		gameObject->physics->SetGravityScale(0.5f + 0.5f * !Input::ButtonDown(SPACE));

		if (Input::OnButtonDown(SPACE) && gameObject->physics->Grounded())
		{
			gameObject->physics->ApplyForce({ 0.0f, -1.0f });
		}

		if (Input::OnButtonDown(F) && attackCooldown <= 0.0f)
		{
			attackCooldown = 0.25f;
			Damage damage{};
			damage.damage = 10;
			damage.armorPierce = 0;
			damage.critChance = 0.1f;
			damage.critMulti = 1.0f;
			damage.knockback = 0.0f;

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

			TimeSlip::Attack(damage, area);
		}

		gameObject->physics->Translate(move);
		health = Math::Min(health + regeneration * (F32)Time::DeltaTime(), maxHealth);
		UI::ChangePercent(healthBar, health / maxHealth);
	}
	else
	{
		deathTimer -= (F32)Time::DeltaTime();

		if (deathTimer <= 0.0f)
		{
			alive = true;
			ignore = false;
			RendererFrontend::DrawGameObject(gameObject);
			SetPosition(spawnPoint);
			health = maxHealth;
			UI::ChangePercent(healthBar, health / maxHealth);
		}
	}
}

void Player::SetPosition(const Vector2& position)
{
	gameObject->transform->SetPosition(position);
}

void Player::DamageResponse()
{
	UI::ChangePercent(healthBar, health / maxHealth);
}