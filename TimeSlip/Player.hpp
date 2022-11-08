#pragma once

#include "Entity.hpp"

struct Vector2;
struct UIBar;

class Player : public Entity
{
private:
	Player(const EntityConfig& config);
	~Player();
	void Destroy() override;
	bool Death() override;

	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update() override;
	void SetPosition(const Vector2& position);

	void DamageResponse() override;

private:
	void Attack(const Damage& damage);

	bool alive;
	F32 deathTimer;
	F32 attackTimer;

	Vector2 spawnPoint;
	UIBar* healthBar;
	UIBar* staminaBar;
	UIBar* manaBar;

	F32 maxStamina;
	F32 stamina;
	F32 staminaRegen;
	F32 maxMana;
	F32 mana;
	F32 manaRegen;

	friend class TimeSlip;
};