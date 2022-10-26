#pragma once

#include <Defines.hpp>

struct Damage
{
	F32 damage;
	F32 armorPierce;
	F32 critChance;
	F32 critMulti;

	//TODO: Debuf applying
};

struct GameObject2D;
struct Vector2;

class Entity
{
protected:
	Entity(const Vector2& position);
	~Entity();
	void Destroy();

	virtual void Update() {}
	bool TakeDamage(const Damage& damage);

protected:
	GameObject2D* gameObject;

	F32 maxHealth;
	F32 health;
	F32 armor;
	F32 damageReduction;

	friend class TimeSlip;
};