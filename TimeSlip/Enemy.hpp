#pragma once

#include <Math/Math.hpp>

#include "Entity.hpp"

struct GameObject2D;
struct Transform2D;

enum EnemyAI
{
	ENEMY_AI_BASIC,
	ENEMY_AI_FLYING,
	ENEMY_AI_RANGED,

	ENEMY_AI_COUNT
};

class Enemy : public Entity
{
private:
	Enemy(const EntityConfig& config, EnemyAI aiType, bool aggressive = true);
	~Enemy();
	void Destroy() override;
	bool Death() override;

	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update() override;
	void BasicAI();
	void FlyingAI();
	void RangedAI();

	void DamageResponse() override;

private:
	EnemyAI aiType;

	Transform2D* target;
	Vector2 lastPos;
	bool startChase;
	bool aggressive;

	F32 attackCooldown;

	friend class TimeSlip;
};