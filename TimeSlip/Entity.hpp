#pragma once

#include <Defines.hpp>
#include <Math/Math.hpp>

struct EntityConfig
{
	F32 maxHealth;
	F32 armor;
	F32 damageReduction;
	F32 knockbackReduction;
	Vector2 position;
	bool ignore;
	F32 despawnRange;
};

struct Damage
{
	F32 damage;
	F32 armorPierce;
	F32 critChance;
	F32 critMulti;
	F32 knockback;

	//TODO: Debuf applying
};

struct GameObject2D;
struct Vector2;

class Entity
{
protected:
	Entity(const EntityConfig& config, bool player = false);
	~Entity();
	virtual void Destroy();
	virtual bool Death() { Destroy(); return true; }

	virtual void Update() {}
	bool TakeDamage(const Damage& damage);
	virtual void DamageResponse() {}

protected:
	GameObject2D* gameObject;

	F32 maxHealth;
	F32 health;
	F32 armor;
	F32 damageReduction;
	F32 knockbackReduction;
	bool facing; //NOTE: false - left, true is right
	bool ignore;
	bool player;

	F32 despawnRange;

	friend class TimeSlip;
};