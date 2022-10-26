#pragma once

#include <Defines.hpp>
#include <Math/Math.hpp>

struct GameObject2D;
struct Transform2D;

enum EnemyAI
{
	ENEMY_AI_BASIC,
	ENEMY_AI_FLYING,
	ENEMY_AI_RANGED,

	ENEMY_AI_COUNT
};

class Enemy
{
private:
	Enemy(const Vector2& position, EnemyAI aiType, bool aggressive = true);
	~Enemy();
	void Destroy();

	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update();
	void BasicAI();
	void FlyingAI();
	void RangedAI();

	bool TakeDamage(F32 amt); //TODO: More advanced damage types

private:
	EnemyAI aiType;

	GameObject2D* gameObject;
	Transform2D* target;
	Vector2 lastPos;
	bool startChase;
	bool aggressive;

	F32 health;

	friend class TimeSlip;
};